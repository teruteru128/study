/**
 * コラッツのdelay(または最大到達bit数)を、山登り・遺伝的アルゴリズム・乱数探索で
 * 最大化してみて、収束曲線をASCIIで描く。
 *
 * collatz_delay_record --landscape が示す通り、ビットパターンがdelayに与える影響は
 * 「最初のM手のうち奇数だった回数j」というスカラー1個に潰れ(相関0.9999)、その
 * 最適解は末尾を全部1にすること = 2^N-1 = collatz_seedが既に出すもの。しかも
 * jが説明できる分散はわずかで、残りには局所構造が無い。
 *
 * したがって探索は 2^N-1 の水準に届かないままプラトーに入るはずで、それを
 * 実際に走らせて確かめるためのツール。best-so-far曲線と、構成解 2^N-1 の水準を
 * 同じ図に描く。
 *
 *   stdout : 収束曲線(TSV)とサマリ。--plot でASCII図も
 *   stderr : 進捗
 */
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <time.h>

#include <gmp.h>

enum
{
    OBJ_STEPS = 0,
    OBJ_MAXBITS = 1
};

typedef struct
{
    const char *name;
    char mark;
    uint64_t *curve; /* curve[i] = i+1回評価した時点でのbest */
    uint64_t best;
    mpz_t best_n;
} result_t;

/* n0のコラッツ軌道を回してdelayまたは最大到達bit数を返す。
   偶数は1bitずつ割らずmpz_scan1で一気にシフトする(hugecollatzと同じ) */
static uint64_t evaluate(const mpz_t n0, int obj, mpz_t x)
{
    mpz_set(x, n0);
    uint64_t steps = 0;
    size_t maxbits = mpz_sizeinbase(x, 2);
    while (mpz_cmp_ui(x, 1) > 0)
    {
        if (mpz_odd_p(x))
        {
            mpz_mul_ui(x, x, 3);
            mpz_add_ui(x, x, 1);
            steps++;
            const size_t b = mpz_sizeinbase(x, 2);
            if (b > maxbits)
            {
                maxbits = b;
            }
        }
        const mp_bitcnt_t k = mpz_scan1(x, 0);
        if (k > 0)
        {
            mpz_fdiv_q_2exp(x, x, k);
            steps += (uint64_t)k;
        }
    }
    return obj == OBJ_STEPS ? steps : (uint64_t)maxbits;
}

static int trailing_ones(const mpz_t n)
{
    return (int)mpz_scan0(n, 0);
}

/* ちょうどbits bitの乱数(最上位bitを立てる) */
static void random_bits(mpz_t out, gmp_randstate_t rs, unsigned long bits)
{
    mpz_urandomb(out, rs, bits);
    mpz_setbit(out, bits - 1);
}

/* 各bitを確率1/bitsで反転する = 反転数はBinomial(bits,1/bits) ≈ Poisson(1)。
   bit数ぶん乱数を引くと大きいbit数で支配的になるので、反転数を一様乱数1回から
   引いてから、その個数だけ位置を選ぶ */
static void mutate(mpz_t n, gmp_randstate_t rs, unsigned long bits)
{
    static const double poisson1_cdf[8] = {0.3678794, 0.7357589, 0.9196986,
                                           0.9810118, 0.9963402, 0.9994058,
                                           0.9999168, 0.9999898};
    const double u = (double)gmp_urandomm_ui(rs, 1000000) / 1000000.0;
    int k = 0;
    while (k < 8 && u > poisson1_cdf[k])
    {
        k++;
    }
    for (int i = 0; i < k; i++)
    {
        mpz_combit(n, gmp_urandomm_ui(rs, bits - 1));
    }
}

/* 評価1回ぶんを記録する。予算を使い切ったら0を返す */
static int record(result_t *r, uint64_t f, const mpz_t n, uint64_t *used,
                  uint64_t budget)
{
    if (*used >= budget)
    {
        return 0;
    }
    if (f > r->best)
    {
        r->best = f;
        mpz_set(r->best_n, n);
    }
    r->curve[*used] = r->best;
    (*used)++;
    return *used < budget;
}

static void run_random(result_t *r, uint64_t budget, unsigned long bits,
                       int obj, gmp_randstate_t rs)
{
    mpz_t n, work;
    mpz_init(n);
    mpz_init(work);
    uint64_t used = 0;
    while (used < budget)
    {
        random_bits(n, rs, bits);
        record(r, evaluate(n, obj, work), n, &used, budget);
    }
    mpz_clear(n);
    mpz_clear(work);
}

/* 最急上昇法。全1bit反転を試して最良へ進み、進めなくなったら再スタート */
static void run_hill(result_t *r, uint64_t budget, unsigned long bits, int obj,
                     gmp_randstate_t rs, uint64_t *restarts)
{
    mpz_t cur, cand, work;
    mpz_init(cur);
    mpz_init(cand);
    mpz_init(work);
    uint64_t used = 0;
    *restarts = 0;

    random_bits(cur, rs, bits);
    uint64_t curf = evaluate(cur, obj, work);
    record(r, curf, cur, &used, budget);

    while (used < budget)
    {
        unsigned long best_bit = 0;
        uint64_t best_f = curf;
        int found = 0;
        /* 最上位bitはbit数を保つため固定 */
        for (unsigned long b = 0; b + 1 < bits && used < budget; b++)
        {
            mpz_set(cand, cur);
            mpz_combit(cand, b);
            const uint64_t f = evaluate(cand, obj, work);
            record(r, f, cand, &used, budget);
            if (f > best_f)
            {
                best_f = f;
                best_bit = b;
                found = 1;
            }
        }
        if (found)
        {
            mpz_combit(cur, best_bit);
            curf = best_f;
        }
        else if (used < budget)
        {
            random_bits(cur, rs, bits);
            curf = evaluate(cur, obj, work);
            record(r, curf, cur, &used, budget);
            (*restarts)++;
        }
    }
    mpz_clear(cur);
    mpz_clear(cand);
    mpz_clear(work);
}

/* 遺伝的アルゴリズム。トーナメント選択・一様交叉・ビット反転突然変異・エリート1 */
static void run_ga(result_t *r, uint64_t budget, unsigned long bits, int obj,
                   gmp_randstate_t rs, unsigned long pop_size,
                   uint64_t *generations)
{
    mpz_t *pop = malloc(pop_size * sizeof(mpz_t));
    mpz_t *next = malloc(pop_size * sizeof(mpz_t));
    uint64_t *fit = malloc(pop_size * sizeof(uint64_t));
    mpz_t work, mask;
    mpz_init(work);
    mpz_init(mask);
    uint64_t used = 0;
    *generations = 0;

    for (unsigned long i = 0; i < pop_size; i++)
    {
        mpz_init(pop[i]);
        mpz_init(next[i]);
        random_bits(pop[i], rs, bits);
        fit[i] = evaluate(pop[i], obj, work);
        record(r, fit[i], pop[i], &used, budget);
    }

    while (used < budget)
    {
        /* エリート1体を残す */
        unsigned long elite = 0;
        for (unsigned long i = 1; i < pop_size; i++)
        {
            if (fit[i] > fit[elite])
            {
                elite = i;
            }
        }
        mpz_set(next[0], pop[elite]);

        for (unsigned long i = 1; i < pop_size; i++)
        {
            /* トーナメント選択(サイズ3)を2回 */
            unsigned long p[2];
            for (int s = 0; s < 2; s++)
            {
                unsigned long bestc = gmp_urandomm_ui(rs, pop_size);
                for (int t = 1; t < 3; t++)
                {
                    const unsigned long c = gmp_urandomm_ui(rs, pop_size);
                    if (fit[c] > fit[bestc])
                    {
                        bestc = c;
                    }
                }
                p[s] = bestc;
            }
            /* 一様交叉: maskのbitが1の位置をp[0]から、0の位置をp[1]から取る */
            mpz_urandomb(mask, rs, bits);
            mpz_and(next[i], pop[p[0]], mask);
            mpz_com(work, mask);
            mpz_and(work, pop[p[1]], work);
            mpz_ior(next[i], next[i], work);
            mutate(next[i], rs, bits);
            mpz_setbit(next[i], bits - 1);
        }

        for (unsigned long i = 0; i < pop_size; i++)
        {
            mpz_set(pop[i], next[i]);
            fit[i] = evaluate(pop[i], obj, work);
            if (!record(r, fit[i], pop[i], &used, budget))
            {
                break;
            }
        }
        (*generations)++;
    }

    for (unsigned long i = 0; i < pop_size; i++)
    {
        mpz_clear(pop[i]);
        mpz_clear(next[i]);
    }
    free(pop);
    free(next);
    free(fit);
    mpz_clear(work);
    mpz_clear(mask);
}

#define PLOT_W 68
#define PLOT_H 20

/* best-so-far曲線と構成解の水準を同じ図に描く。x軸は評価回数の対数 */
static void plot(result_t *res, int nres, uint64_t budget, uint64_t baseline,
                 const char *baseline_name, const char *ylabel)
{
    uint64_t hi = baseline, lo = UINT64_MAX;
    for (int i = 0; i < nres; i++)
    {
        if (res[i].best > hi)
        {
            hi = res[i].best;
        }
        if (res[i].curve[0] < lo)
        {
            lo = res[i].curve[0];
        }
    }
    if (hi <= lo)
    {
        hi = lo + 1;
    }
    /* 上下に少し余白 */
    const double span = (double)(hi - lo);
    const double top = (double)hi + span * 0.08;
    const double bot = (double)lo - span * 0.08;

    static char grid[PLOT_H][PLOT_W + 1];
    memset(grid, ' ', sizeof(grid));
    for (int y = 0; y < PLOT_H; y++)
    {
        grid[y][PLOT_W] = '\0';
    }

    const double logmax = log10((double)budget);
    for (int i = 0; i < nres; i++)
    {
        for (int x = 0; x < PLOT_W; x++)
        {
            /* x列が表す評価回数(対数等分) */
            const double e = pow(10.0, logmax * (x + 1) / PLOT_W);
            uint64_t idx = (uint64_t)e;
            if (idx < 1)
            {
                idx = 1;
            }
            if (idx > budget)
            {
                idx = budget;
            }
            const double v = (double)res[i].curve[idx - 1];
            int y = (int)((top - v) / (top - bot) * (PLOT_H - 1) + 0.5);
            if (y < 0)
            {
                y = 0;
            }
            if (y >= PLOT_H)
            {
                y = PLOT_H - 1;
            }
            grid[y][x] = res[i].mark;
        }
    }
    /* 構成解の水準 */
    {
        int y = (int)((top - (double)baseline) / (top - bot) * (PLOT_H - 1) + 0.5);
        if (y < 0)
        {
            y = 0;
        }
        if (y >= PLOT_H)
        {
            y = PLOT_H - 1;
        }
        for (int x = 0; x < PLOT_W; x++)
        {
            if (grid[y][x] == ' ')
            {
                grid[y][x] = '=';
            }
        }
    }

    printf("\n%s\n", ylabel);
    for (int y = 0; y < PLOT_H; y++)
    {
        const double v = top - (top - bot) * y / (PLOT_H - 1);
        printf("%9.0f |%s\n", v, grid[y]);
    }
    printf("%9s +", "");
    for (int x = 0; x < PLOT_W; x++)
    {
        putchar('-');
    }
    putchar('\n');

    /* 目盛りは一度バッファに置いてから出す(桁数が違うとprintfの詰めが狂うため) */
    char axis[PLOT_W + 8];
    memset(axis, ' ', sizeof(axis));
    axis[sizeof(axis) - 1] = '\0';
    for (int p = 0; p <= (int)logmax; p++)
    {
        char tick[8];
        const int len = snprintf(tick, sizeof(tick), "1e%d", p);
        /* 列xが10^pに対応する: 10^(logmax*(x+1)/PLOT_W) = 10^p */
        int x = (int)(PLOT_W * p / logmax) - 1;
        if (x < 0)
        {
            x = 0;
        }
        if (x + len < PLOT_W + 4)
        {
            memcpy(axis + x, tick, (size_t)len);
        }
    }
    for (int x = PLOT_W + 6; x >= 0 && axis[x] == ' '; x--)
    {
        axis[x] = '\0';
    }
    printf("%10s%s\n", "", axis);
    printf("%10s評価回数(対数)\n", "");

    printf("\n凡例: ");
    for (int i = 0; i < nres; i++)
    {
        printf("%c=%s  ", res[i].mark, res[i].name);
    }
    printf("= %s\n", baseline_name);
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [OPTIONS]\n"
            "  -b, --bits N        探索する数のbit数(既定: 256)\n"
            "  -e, --evals N       1手法あたりの評価回数(既定: 200000)\n"
            "  -o, --objective O   steps または max (既定: steps)\n"
            "  -p, --population N  GAの個体数(既定: 100)\n"
            "  -s, --seed S        乱数シード(既定: /dev/urandom)\n"
            "  -P, --plot          収束曲線をASCIIで描く\n"
            "  -c, --curve         収束曲線をTSVで出す\n",
            argv0);
}

int main(int argc, char **argv)
{
    unsigned long bits = 256;
    uint64_t budget = 200000;
    unsigned long pop_size = 100;
    unsigned long seed = 0;
    int have_seed = 0, obj = OBJ_STEPS, do_plot = 0, do_curve = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *a = argv[i];
        if ((strcmp(a, "-b") == 0 || strcmp(a, "--bits") == 0) && i + 1 < argc)
        {
            bits = strtoul(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-e") == 0 || strcmp(a, "--evals") == 0) &&
                 i + 1 < argc)
        {
            budget = strtoull(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-o") == 0 || strcmp(a, "--objective") == 0) &&
                 i + 1 < argc)
        {
            const char *v = argv[++i];
            if (strcmp(v, "steps") == 0)
            {
                obj = OBJ_STEPS;
            }
            else if (strcmp(v, "max") == 0)
            {
                obj = OBJ_MAXBITS;
            }
            else
            {
                fprintf(stderr, "-o は steps か max です\n");
                return EXIT_FAILURE;
            }
        }
        else if ((strcmp(a, "-p") == 0 || strcmp(a, "--population") == 0) &&
                 i + 1 < argc)
        {
            pop_size = strtoul(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-s") == 0 || strcmp(a, "--seed") == 0) &&
                 i + 1 < argc)
        {
            seed = strtoul(argv[++i], NULL, 10);
            have_seed = 1;
        }
        else if (strcmp(a, "-P") == 0 || strcmp(a, "--plot") == 0)
        {
            do_plot = 1;
        }
        else if (strcmp(a, "-c") == 0 || strcmp(a, "--curve") == 0)
        {
            do_curve = 1;
        }
        else
        {
            usage(argv[0]);
            return (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0)
                       ? EXIT_SUCCESS
                       : EXIT_FAILURE;
        }
    }
    if (bits < 8 || budget < 100 || pop_size < 4)
    {
        fprintf(stderr, "-b は8以上、-e は100以上、-p は4以上にしてください\n");
        return EXIT_FAILURE;
    }
    if (!have_seed &&
        getrandom(&seed, sizeof(seed), 0) != (ssize_t)sizeof(seed))
    {
        seed = (unsigned long)time(NULL);
    }

    gmp_randstate_t rs;
    gmp_randinit_mt(rs);
    gmp_randseed_ui(rs, seed);

    result_t res[3] = {{"random", 'r', NULL, 0, {{0}}},
                       {"hill", 'h', NULL, 0, {{0}}},
                       {"ga", 'g', NULL, 0, {{0}}}};
    for (int i = 0; i < 3; i++)
    {
        res[i].curve = malloc(budget * sizeof(uint64_t));
        if (!res[i].curve)
        {
            fprintf(stderr, "メモリを確保できませんでした\n");
            return EXIT_FAILURE;
        }
        mpz_init(res[i].best_n);
    }

    fprintf(stderr, "# bits=%lu evals=%" PRIu64 " objective=%s pop=%lu seed=%lu\n",
            bits, budget, obj == OBJ_STEPS ? "steps" : "max", pop_size, seed);

    uint64_t restarts = 0, generations = 0;
    const double t0 = (double)clock() / CLOCKS_PER_SEC;
    fprintf(stderr, "# random...\n");
    run_random(&res[0], budget, bits, obj, rs);
    fprintf(stderr, "# hill...\n");
    run_hill(&res[1], budget, bits, obj, rs, &restarts);
    fprintf(stderr, "# ga...\n");
    run_ga(&res[2], budget, bits, obj, rs, pop_size, &generations);
    const double elapsed = (double)clock() / CLOCKS_PER_SEC - t0;

    /* 構成解 2^bits-1 */
    mpz_t allones, work;
    mpz_init(allones);
    mpz_init(work);
    mpz_set_ui(allones, 1);
    mpz_mul_2exp(allones, allones, bits);
    mpz_sub_ui(allones, allones, 1);
    const uint64_t base = evaluate(allones, obj, work);

    const char *label = obj == OBJ_STEPS ? "delay(総ステップ数)"
                                         : "最大到達bit数";
    printf("method\tbest\ttrailing_ones\tvs_baseline\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%s\t%" PRIu64 "\t%d\t%.1f%%\n", res[i].name, res[i].best,
               trailing_ones(res[i].best_n), 100.0 * res[i].best / base);
    }
    printf("2^%lu-1(構成)\t%" PRIu64 "\t%lu\t100.0%%\n", bits, base, bits);
    fprintf(stderr, "# hill restarts=%" PRIu64 " ga generations=%" PRIu64
                    " elapsed=%.1fs\n",
            restarts, generations, elapsed);

    if (do_curve)
    {
        printf("\nevals\trandom\thill\tga\n");
        for (uint64_t e = 1; e <= budget; e *= 2)
        {
            printf("%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n", e,
                   res[0].curve[e - 1], res[1].curve[e - 1],
                   res[2].curve[e - 1]);
        }
    }
    if (do_plot)
    {
        char bname[64];
        snprintf(bname, sizeof(bname), "2^%lu-1(構成)", bits);
        plot(res, 3, budget, base, bname, label);
    }

    for (int i = 0; i < 3; i++)
    {
        free(res[i].curve);
        mpz_clear(res[i].best_n);
    }
    mpz_clear(allones);
    mpz_clear(work);
    gmp_randclear(rs);
    return EXIT_SUCCESS;
}
