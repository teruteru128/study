/**
 * コラッツのdelay記録を、剰余類の一括ジャンプで探す。
 *
 * collatz_delay_record は 2^b 要素のメモ化配列を持つため 2^32 が天井(8GiB)。
 * こちらはパリティベクトル定理を使ってメモ化配列を小さく保ち、その先へ進む。
 *
 * n = q*2^K + r と書くと、n の最初の K ステップ(ショートカット写像)の偶奇列は
 * r だけで決まる。その K 手ぶんを O(1) で進められる:
 *
 *   n = q*2^K + r  --K手-->  3^j(r) * q + t(r)      実ステップ数は K + j(r)
 *
 * ここで j(r) は最初のK手のうち奇数だった回数、t(r) は r 自身をK手進めた値。
 * j(r) と t(r) は 2^K 要素の表に前計算しておく。1回の表引きで K + j 手進むので、
 * 軌道を1手ずつ辿るより速い。
 *
 * delay は「2^T 未満に落ちるまでジャンプで辿り、そこからはメモ化表を引く」で
 * 厳密に求まる。T は探索範囲と独立に選べるので、メモリが探索範囲を縛らない。
 *
 *   stdout : bit数ごとの記録(TSV、collatz_delay_recordと同じ列)
 *   stderr : 進捗とメモリ使用量
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t delay_t;

/* r を K 手進めた結果。t は 3^j 程度まで大きくなるので64bitで持つ */
typedef struct
{
    uint64_t t;
    uint8_t j;
} jump_t;

static uint64_t pow3[64];

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [OPTIONS]\n"
            "  -b, --max-bits B  探索する最大bit数(既定: 32)\n"
            "  -m, --min-bits M  探索する最小bit数(既定: 3)\n"
            "  -k, --jump-bits K 一度に進めるステップ数(既定: 16)\n"
            "                    表は 2^K * 9バイト\n"
            "  -t, --memo-bits T メモ化する範囲(既定: 26)\n"
            "                    配列は 2^T * 2バイト。K以上にすること\n"
            "  -j, --min-odd J   最初のK手の奇数回数がJ未満の類を飛ばす\n"
            "                    (既定: 0 = 全探索)。0以外は網羅的ではない\n",
            argv0);
}

/* 剰余類テーブルを作る。r を K 手進めながら奇数回数を数える */
static jump_t *build_jump_table(int k)
{
    const size_t n = (size_t)1 << k;
    jump_t *tab = malloc(n * sizeof(jump_t));
    if (!tab)
    {
        return NULL;
    }
    for (size_t r = 0; r < n; r++)
    {
        uint64_t x = r;
        int j = 0;
        for (int i = 0; i < k; i++)
        {
            if (x & 1)
            {
                j++;
                x = (3 * x + 1) / 2;
            }
            else
            {
                x /= 2;
            }
        }
        tab[r].t = x;
        tab[r].j = (uint8_t)j;
    }
    return tab;
}

/* 2^T 未満のメモ化表。n未満に落ちるまで辿ってその値のdelayを足す */
static delay_t *build_memo(int t)
{
    const uint64_t limit = UINT64_C(1) << t;
    delay_t *memo = malloc(limit * sizeof(delay_t));
    if (!memo)
    {
        return NULL;
    }
    memo[1] = 0;
    for (uint64_t n = 2; n < limit; n++)
    {
        uint64_t x = n;
        uint32_t s = 0;
        while (x >= n)
        {
            x = (x & 1) ? 3 * x + 1 : x >> 1;
            s++;
        }
        memo[n] = (delay_t)(s + memo[x]);
    }
    return memo;
}

/* n のdelayを厳密に求める。溢れたら 0 を返す(呼び出し側で打ち切る) */
static uint64_t delay_of(uint64_t n, const jump_t *tab, int k,
                         const delay_t *memo, uint64_t memo_limit,
                         int *overflow)
{
    const uint64_t mask = (UINT64_C(1) << k) - 1;
    uint64_t steps = 0;
    uint64_t v = n;
    while (v >= memo_limit)
    {
        const jump_t *e = &tab[v & mask];
        uint64_t hi;
        /* v = q*2^K + r  ->  3^j * q + t */
        if (__builtin_mul_overflow(pow3[e->j], v >> k, &hi) ||
            __builtin_add_overflow(hi, e->t, &v))
        {
            *overflow = 1;
            return 0;
        }
        steps += (uint64_t)k + e->j;
        if (v == 0)
        {
            *overflow = 1;
            return 0;
        }
    }
    return steps + memo[v];
}

int main(int argc, char **argv)
{
    int max_bits = 32, min_bits = 3, k = 16, t = 26, min_odd = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *a = argv[i];
        if ((strcmp(a, "-b") == 0 || strcmp(a, "--max-bits") == 0) && i + 1 < argc)
        {
            max_bits = atoi(argv[++i]);
        }
        else if ((strcmp(a, "-m") == 0 || strcmp(a, "--min-bits") == 0) && i + 1 < argc)
        {
            min_bits = atoi(argv[++i]);
        }
        else if ((strcmp(a, "-k") == 0 || strcmp(a, "--jump-bits") == 0) && i + 1 < argc)
        {
            k = atoi(argv[++i]);
        }
        else if ((strcmp(a, "-t") == 0 || strcmp(a, "--memo-bits") == 0) && i + 1 < argc)
        {
            t = atoi(argv[++i]);
        }
        else if ((strcmp(a, "-j") == 0 || strcmp(a, "--min-odd") == 0) && i + 1 < argc)
        {
            min_odd = atoi(argv[++i]);
        }
        else
        {
            usage(argv[0]);
            return (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) ? EXIT_SUCCESS
                                                                      : EXIT_FAILURE;
        }
    }
    if (k < 4 || k > 24)
    {
        fprintf(stderr, "-k は4〜24で指定してください\n");
        return EXIT_FAILURE;
    }
    if (t < k || t > 32)
    {
        fprintf(stderr, "-t は %d〜32(K以上)で指定してください\n", k);
        return EXIT_FAILURE;
    }
    if (max_bits < 2 || max_bits > 48 || min_bits < 2 || min_bits > max_bits)
    {
        fprintf(stderr, "-b は2〜48、-m は2〜-b で指定してください\n");
        return EXIT_FAILURE;
    }
    if (min_odd < 0 || min_odd > k)
    {
        fprintf(stderr, "-j は0〜%dで指定してください\n", k);
        return EXIT_FAILURE;
    }

    pow3[0] = 1;
    for (int i = 1; i < 41; i++)
    {
        pow3[i] = pow3[i - 1] * 3;
    }

    fprintf(stderr, "# ジャンプ表: 2^%d * %zuB = %.1fMiB / メモ化: 2^%d * %zuB = %.1fMiB\n",
            k, sizeof(jump_t), (double)((size_t)1 << k) * sizeof(jump_t) / 1048576.0,
            t, sizeof(delay_t), (double)((size_t)1 << t) * sizeof(delay_t) / 1048576.0);
    if (min_odd > 0)
    {
        fprintf(stderr, "# 注意: -j %d は網羅的ではありません(記録を見落としうる)\n",
                min_odd);
    }

    jump_t *tab = build_jump_table(k);
    delay_t *memo = build_memo(t);
    if (!tab || !memo)
    {
        fprintf(stderr, "メモリを確保できませんでした\n");
        return EXIT_FAILURE;
    }
    const uint64_t memo_limit = UINT64_C(1) << t;
    const uint64_t mask = (UINT64_C(1) << k) - 1;

    printf("bits\tdelay\tper_bit\tvalue\tbinary\n");
    int overflow = 0;
    for (int b = min_bits; b <= max_bits && !overflow; b++)
    {
        const uint64_t lo = UINT64_C(1) << (b - 1);
        const uint64_t hi = UINT64_C(1) << b;
        uint64_t best = lo, bd = 0;
        if (hi <= memo_limit)
        {
            /* メモ化の範囲内なら表を引くだけ */
            for (uint64_t n = lo; n < hi; n++)
            {
                if (memo[n] > bd)
                {
                    bd = memo[n];
                    best = n;
                }
            }
        }
        else
        {
            for (uint64_t n = lo; n < hi; n++)
            {
                if (min_odd > 0 && tab[n & mask].j < min_odd)
                {
                    continue;
                }
                const uint64_t d = delay_of(n, tab, k, memo, memo_limit, &overflow);
                if (overflow)
                {
                    fprintf(stderr, "# %" PRIu64 " で64bitを溢れました。打ち切ります\n", n);
                    break;
                }
                if (d > bd)
                {
                    bd = d;
                    best = n;
                }
            }
        }
        if (overflow)
        {
            break;
        }
        printf("%d\t%" PRIu64 "\t%.2f\t%" PRIu64 "\t", b, bd, (double)bd / b, best);
        for (int i = b - 1; i >= 0; i--)
        {
            putchar((best >> i & 1) ? '1' : '0');
        }
        putchar('\n');
        fflush(stdout);
        /* 進捗は\rで上書きせず1行ずつ出す。stdoutと混ぜてログに落としたときに
           結果の行を巻き込まないため */
        fprintf(stderr, "# %dbit 完了\n", b);
        fflush(stderr);
    }

    free(tab);
    free(memo);
    return overflow ? EXIT_FAILURE : EXIT_SUCCESS;
}
