/**
 * 巨大な整数に対してコラッツ(3n+1)のシーケンスを計算し、
 * 総ステップ数と最大到達値を求める。
 *
 * 入力は既定で16進(rsa_extract_primesの出力をそのまま食える)。
 * 100万bit級だと1000万ステップ規模になるため、途中経過を定期的にstderrへ出す。
 *
 *   stdout : 最大到達値(16進、1行)
 *   stderr : 途中経過とサマリ
 *
 * 偶数を1bitずつ割るのではなく、mpz_scan1()で立っている最下位bitまで
 * 一気にシフトする(ステップ数はシフト量ぶん加算するので結果は同じ)。
 * 巨大数演算の回数がおよそ1/3になる。
 */
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gmp.h>

static volatile sig_atomic_t report_now = 0;
static volatile sig_atomic_t stop_now = 0;

static void on_sigusr1(int sig)
{
    (void)sig;
    report_now = 1;
}

static void on_sigint(int sig)
{
    (void)sig;
    stop_now = 1;
}

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [OPTIONS] [NUMBER]\n"
            "  NUMBER を省略すると標準入力から1行読む\n"
            "  -d, --dec           入力を10進として解釈する(既定: 16進)\n"
            "  -i, --interval N    途中経過を出す間隔(ステップ数, 既定 1000000)\n"
            "  -q, --quiet         途中経過を出さない\n"
            "  -o, --output FILE   最大到達値の出力先(既定: 標準出力)\n"
            "  -s, --summary-only  最大到達値そのものは出力しない\n"
            "      --max-steps N   Nステップで打ち切る(既定: 無制限)\n"
            "\n"
            "  実行中に SIGUSR1 を送ると即座に途中経過を出す。\n"
            "  SIGINT (Ctrl-C) で打ち切り、そこまでのサマリを出して終了する。\n",
            argv0);
}

/* 1行読む。改行と空白を落として返す。失敗時NULL。 */
static char *read_line(FILE *fp)
{
    size_t cap = 4096;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf)
    {
        return NULL;
    }
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n')
    {
        if (c == ' ' || c == '\t' || c == '\r')
        {
            continue;
        }
        if (len + 1 >= cap)
        {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }
    if (len == 0)
    {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

static void print_progress(const char *tag, uint64_t steps, const mpz_t n,
                           size_t start_bits, size_t max_bits, double elapsed)
{
    size_t bits = mpz_sizeinbase(n, 2);
    double done = 0.0;
    if (start_bits > 0 && bits < start_bits)
    {
        done = (double)(start_bits - bits) * 100.0 / (double)start_bits;
    }
    fprintf(stderr,
            "[%s] steps=%" PRIu64 " bits=%zu max_bits=%zu progress~%.2f%% "
            "elapsed=%.1fs\n",
            tag, steps, bits, max_bits, done, elapsed);
    fflush(stderr);
}

int main(int argc, char **argv)
{
    int base = 16;
    int quiet = 0;
    int summary_only = 0;
    uint64_t interval = 1000000;
    uint64_t max_steps = 0; /* 0なら無制限 */
    const char *output = NULL;
    const char *number = NULL;

    for (int i = 1; i < argc; i++)
    {
        const char *a = argv[i];
        if (strcmp(a, "-d") == 0 || strcmp(a, "--dec") == 0)
        {
            base = 10;
        }
        else if (strcmp(a, "-q") == 0 || strcmp(a, "--quiet") == 0)
        {
            quiet = 1;
        }
        else if (strcmp(a, "-s") == 0 || strcmp(a, "--summary-only") == 0)
        {
            summary_only = 1;
        }
        else if ((strcmp(a, "-i") == 0 || strcmp(a, "--interval") == 0) &&
                 i + 1 < argc)
        {
            interval = strtoull(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-o") == 0 || strcmp(a, "--output") == 0) &&
                 i + 1 < argc)
        {
            output = argv[++i];
        }
        else if (strcmp(a, "--max-steps") == 0 && i + 1 < argc)
        {
            max_steps = strtoull(argv[++i], NULL, 10);
        }
        else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0)
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (a[0] == '-' && a[1] != '\0')
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else if (!number)
        {
            number = a;
        }
        else
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (interval == 0)
    {
        interval = 1;
    }

    char *from_stdin = NULL;
    if (!number)
    {
        from_stdin = read_line(stdin);
        if (!from_stdin)
        {
            fprintf(stderr, "標準入力から数値を読めませんでした\n");
            return EXIT_FAILURE;
        }
        number = from_stdin;
    }

    /* 10進ファイルを16進として読んでしまう事故のチェック(even-numberファイルと同じ罠) */
    if (base == 16 && strcspn(number, "abcdefABCDEF") == strlen(number))
    {
        fprintf(stderr,
                "warning: 入力にa-fが1文字も含まれていません。"
                "10進数を16進として読んでいませんか? (-d で10進指定)\n");
    }

    mpz_t n, max;
    mpz_init(n);
    if (mpz_set_str(n, number, base) != 0)
    {
        fprintf(stderr, "%d進数として解釈できません\n", base);
        mpz_clear(n);
        free(from_stdin);
        return EXIT_FAILURE;
    }
    free(from_stdin);
    from_stdin = NULL;
    number = NULL;

    if (mpz_sgn(n) <= 0)
    {
        fprintf(stderr, "正の整数を指定してください\n");
        mpz_clear(n);
        return EXIT_FAILURE;
    }

    size_t start_bits = mpz_sizeinbase(n, 2);
    mpz_init_set(max, n);
    uint64_t steps = 0;
    uint64_t max_step = 0;
    uint64_t next_report = interval;
    double t0 = now_sec();

    signal(SIGUSR1, on_sigusr1);
    signal(SIGINT, on_sigint);

    if (!quiet)
    {
        fprintf(stderr, "# start: %zubit\n", start_bits);
    }

    int aborted = 0;
    while (mpz_cmp_ui(n, 1) > 0)
    {
        if (mpz_odd_p(n))
        {
            mpz_mul_ui(n, n, 3);
            mpz_add_ui(n, n, 1);
            steps++;
            /* 最大値は必ず3n+1の直後に現れる(シフトは必ず減少するため) */
            if (mpz_cmp(n, max) > 0)
            {
                mpz_set(max, n);
                max_step = steps;
            }
        }

        /* 立っている最下位bitまで一気にシフト = 2で割る操作をまとめて行う */
        mp_bitcnt_t k = mpz_scan1(n, 0);
        if (k > 0)
        {
            mpz_fdiv_q_2exp(n, n, k);
            steps += (uint64_t)k;
        }

        if (report_now)
        {
            report_now = 0;
            print_progress("signal", steps, n, start_bits,
                           mpz_sizeinbase(max, 2), now_sec() - t0);
        }
        if (steps >= next_report)
        {
            if (!quiet)
            {
                print_progress("progress", steps, n, start_bits,
                               mpz_sizeinbase(max, 2), now_sec() - t0);
            }
            while (next_report <= steps)
            {
                next_report += interval;
            }
        }
        if (stop_now)
        {
            fprintf(stderr, "interrupted\n");
            aborted = 1;
            break;
        }
        if (max_steps != 0 && steps >= max_steps)
        {
            fprintf(stderr, "max-steps(%" PRIu64 ")に到達したため打ち切り\n",
                    max_steps);
            aborted = 1;
            break;
        }
    }

    double elapsed = now_sec() - t0;
    fprintf(stderr,
            "steps=%" PRIu64 "\n"
            "max_bits=%zu\n"
            "max_step=%" PRIu64 "\n"
            "reached_one=%s\n"
            "elapsed=%.3fs\n",
            steps, mpz_sizeinbase(max, 2), max_step, aborted ? "no" : "yes",
            elapsed);

    int ret = EXIT_SUCCESS;
    if (!summary_only)
    {
        FILE *out = stdout;
        if (output)
        {
            out = fopen(output, "w");
            if (!out)
            {
                fprintf(stderr, "cannot open '%s': %s\n", output,
                        strerror(errno));
                ret = EXIT_FAILURE;
            }
        }
        if (out)
        {
            mpz_out_str(out, 16, max);
            fputc('\n', out);
            if (out != stdout)
            {
                fclose(out);
            }
        }
    }

    mpz_clear(n);
    mpz_clear(max);
    return aborted ? EXIT_FAILURE : ret;
}
