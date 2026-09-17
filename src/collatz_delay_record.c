/**
 * bit数ごとにコラッツの総ステップ数(delay)が最大の初期値を全探索で求める。
 *
 * collatz_seed が構成できるのは「末尾の1連長Kが保証する上昇区間」までで、
 * それ以降の挙動は構成では制御できない(Terrasのパリティベクトル定理により、
 * 最初のm手の偶奇列はn mod 2^mと1対1に対応する = mbitでm手ぶんしか指定できない)。
 * 地平線の向こう側で何が起きるかは全探索でしか分からないので、その全探索を行う。
 *
 * delay(n)は「n未満に落ちるまで辿って、その値のdelayを足す」メモ化で求める。
 * 昇順に走査するので、落ちた先のdelayは必ず計算済み。
 *
 * --stats を付けると平均・標準偏差も出す。delayの平均はbit数に比例して増えるのに
 * 標準偏差は√bit でしか増えない(sd_per_sqrt_bitが一定)ため、大きい数ほど分布は
 * 相対的に鋭くなり、乱数で記録を引き当てるのは急速に絶望的になる。その度合いを
 * 「N個引いたときの最大値が平均から何σ離れるか」として最大bit数について出す。
 *
 * --landscape は「ビットパターンからdelayを予測できるか = 探索が学習できる構造が
 * あるか」を測る。下位Mbitの剰余類ごとの平均delayは、その類の最初のM手のうち
 * 奇数ステップだった回数jだけでほぼ決まる(M=10で相関0.9999)。つまり構成で稼げる
 * ぶんはスカラー1個に潰れ、その最適解は「末尾を全部1にする」= collatz_seedが
 * 既に出すもの。残りの分散には局所構造が無く、記録保持者の1bit近傍のdelayは
 * 一気に全体平均付近まで落ちる。山登りやGAが末尾全部1の先へ進めないことの根拠。
 *
 *   stdout : bit数ごとの記録(TSV)。--stats/--landscapeでは続けて追加の表
 *   stderr : 進捗とメモリ使用量
 */
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* n < 2^32 での最大delayは1100程度なのでuint16で足りる */
typedef uint16_t delay_t;

/* 2^32を超えると軌道の最大値が3*x+1でuint64を溢れさせうる。
   どのみち2^32でもメモ化配列が8GiBになるのでここで頭打ちにする */
#define MAX_BITS_LIMIT 32

static int trailing_ones(uint64_t n)
{
    int c = 0;
    while (n & 1)
    {
        c++;
        n >>= 1;
    }
    return c;
}

/* 剰余類rから始めてMステップ(ショートカット写像)のうち奇数だった回数 */
static int odd_steps(uint32_t r, int m)
{
    uint64_t x = r;
    int j = 0;
    for (int i = 0; i < m; i++)
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
    return j;
}

/* ビットパターンからdelayがどこまで予測できるかを測る */
static void print_landscape(const delay_t *delay, int bits, uint64_t lo,
                            uint64_t hi)
{
    const uint64_t cnt = hi - lo;
    double gm = 0.0;
    for (uint64_t n = lo; n < hi; n++)
    {
        gm += delay[n];
    }
    gm /= (double)cnt;
    double sst = 0.0;
    for (uint64_t n = lo; n < hi; n++)
    {
        const double v = delay[n] - gm;
        sst += v * v;
    }

    /* (1) 下位Mbitの剰余類ごとの平均delayは、その類のjでどこまで説明できるか */
    /* 1類あたり最低64個を確保する(群平均のSEが約8になり、jによる変動幅
       およそ125に対して十分小さい)。それより粗い解像度では表を出さない */
    int max_m = bits - 7;
    if (max_m > 12)
    {
        max_m = 12;
    }
    if (max_m < 2)
    {
        fprintf(stderr,
                "# bit数が小さすぎて剰余類の統計が取れないため、"
                "剰余類の表は省略します\n");
    }
    else
    {
        printf("\n# %dbit: 下位Mbitの剰余類平均 vs j(最初のM手の奇数回数)\n",
               bits);
        printf("low_bits\tclasses\tr\tslope\tmean_j0\tmean_jmax\t"
               "var_explained\n");
    }
    const size_t nmax = (size_t)1 << (max_m < 2 ? 2 : max_m);
    double *gsum = malloc(nmax * sizeof(double));
    uint64_t *gcnt = malloc(nmax * sizeof(uint64_t));
    if (!gsum || !gcnt)
    {
        free(gsum);
        free(gcnt);
        return;
    }
    for (int m = 2; m <= max_m; m += 2)
    {
        const uint32_t nclass = 1U << m;
        for (uint32_t r = 0; r < nclass; r++)
        {
            gsum[r] = 0.0;
            gcnt[r] = 0;
        }
        for (uint64_t n = lo; n < hi; n++)
        {
            const uint32_t r = (uint32_t)(n & (nclass - 1));
            gsum[r] += delay[n];
            gcnt[r]++;
        }
        /* jと群平均の相関・回帰、および群平均が説明する分散の割合 */
        double sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0, ssb = 0;
        uint32_t k = 0;
        for (uint32_t r = 0; r < nclass; r++)
        {
            if (gcnt[r] == 0)
            {
                continue;
            }
            const double x = odd_steps(r, m);
            const double y = gsum[r] / (double)gcnt[r];
            sx += x;
            sy += y;
            sxx += x * x;
            syy += y * y;
            sxy += x * y;
            ssb += (double)gcnt[r] * (y - gm) * (y - gm);
            k++;
        }
        const double cov = sxy / k - (sx / k) * (sy / k);
        const double vx = sxx / k - (sx / k) * (sx / k);
        const double vy = syy / k - (sy / k) * (sy / k);
        printf("%d\t%u\t%.4f\t%.2f\t%.1f\t%.1f\t%.2f%%\n", m, k,
               (vx > 0 && vy > 0) ? cov / sqrt(vx * vy) : 0.0,
               vx > 0 ? cov / vx : 0.0, gsum[0] / (double)gcnt[0],
               gsum[nclass - 1] / (double)gcnt[nclass - 1],
               sst > 0 ? 100.0 * ssb / sst : 0.0);
    }
    free(gsum);
    free(gcnt);

    /* (2) 1bit反転した近傍とのdelay相関。局所構造があれば山登りが効く */
    printf("\n# %dbit: 1bit反転した近傍とのdelay相関\n", bits);
    printf("flip_bit\tsamples\tr\n");
    const uint64_t stride = 997; /* 全走査はキャッシュミスで遅いので間引く */
    for (int bit = 0; bit < bits; bit++)
    {
        double sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0;
        uint64_t c = 0;
        for (uint64_t n = lo; n < hi; n += stride)
        {
            const uint64_t nb = n ^ (UINT64_C(1) << bit);
            if (nb < lo || nb >= hi)
            {
                continue;
            }
            const double x = delay[n], y = delay[nb];
            sx += x;
            sy += y;
            sxx += x * x;
            syy += y * y;
            sxy += x * y;
            c++;
        }
        if (c < 2)
        {
            continue;
        }
        const double cov = sxy / c - (sx / c) * (sy / c);
        const double v1 = sxx / c - (sx / c) * (sx / c);
        const double v2 = syy / c - (sy / c) * (sy / c);
        printf("%d\t%" PRIu64 "\t%+.4f\n", bit, c,
               (v1 > 0 && v2 > 0) ? cov / sqrt(v1 * v2) : 0.0);
    }

    /* (3) 上位群の1bit近傍は優秀か。山登りが頂上から落ちるかを直接見る */
    const size_t hsize = (size_t)1 << (8 * sizeof(delay_t));
    uint64_t *hist = calloc(hsize, sizeof(uint64_t));
    if (!hist)
    {
        return;
    }
    for (uint64_t n = lo; n < hi; n++)
    {
        hist[delay[n]]++;
    }
    /* 範囲が狭いときに上位群が範囲の大半を占めてしまわないようにする */
    uint64_t want = cnt / 16;
    if (want > 1000)
    {
        want = 1000;
    }
    if (want < 1)
    {
        want = 1;
    }
    uint64_t acc = 0;
    size_t th = hsize - 1;
    while (th > 0 && acc < want)
    {
        acc += hist[th];
        if (acc >= want)
        {
            break;
        }
        th--;
    }
    free(hist);

    double top_sum = 0, nb_sum = 0;
    uint64_t top_c = 0, nb_c = 0;
    for (uint64_t n = lo; n < hi; n++)
    {
        if (delay[n] < th)
        {
            continue;
        }
        top_sum += delay[n];
        top_c++;
        for (int bit = 0; bit < bits; bit++)
        {
            const uint64_t nb = n ^ (UINT64_C(1) << bit);
            if (nb >= lo && nb < hi)
            {
                nb_sum += delay[nb];
                nb_c++;
            }
        }
    }
    printf("\n# %dbit: 上位群とその1bit近傍の平均delay\n", bits);
    printf("group\tcount\tmean\n");
    printf("top\t%" PRIu64 "\t%.1f\n", top_c, top_sum / (double)top_c);
    printf("neighbors\t%" PRIu64 "\t%.1f\n", nb_c, nb_sum / (double)nb_c);
    printf("all\t%" PRIu64 "\t%.1f\n", cnt, gm);
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [OPTIONS]\n"
            "  -b, --max-bits B  探索する最大bit数(既定: 26, 上限: %d)\n"
            "  -m, --min-bits M  探索する最小bit数(既定: 3)\n"
            "  -v, --verbose     末尾1連長・2^b-1のdelayも出す\n"
            "  -S, --stats       平均・標準偏差・記録のσ位置と、最大bit数に\n"
            "                    ついてのbest-of-N表も出す\n"
            "  -L, --landscape   最大bit数について、ビットパターンからdelayが\n"
            "                    どこまで予測できるかを測る\n"
            "\n"
            "  メモ化配列に 2^B * %zu バイト使う(B=26で128MiB, B=30で2GiB)。\n",
            argv0, MAX_BITS_LIMIT, sizeof(delay_t));
}

int main(int argc, char **argv)
{
    int max_bits = 26;
    int min_bits = 3;
    int verbose = 0;
    int stats = 0;
    int landscape = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *a = argv[i];
        if ((strcmp(a, "-b") == 0 || strcmp(a, "--max-bits") == 0) &&
            i + 1 < argc)
        {
            max_bits = atoi(argv[++i]);
        }
        else if ((strcmp(a, "-m") == 0 || strcmp(a, "--min-bits") == 0) &&
                 i + 1 < argc)
        {
            min_bits = atoi(argv[++i]);
        }
        else if (strcmp(a, "-v") == 0 || strcmp(a, "--verbose") == 0)
        {
            verbose = 1;
        }
        else if (strcmp(a, "-S") == 0 || strcmp(a, "--stats") == 0)
        {
            stats = 1;
        }
        else if (strcmp(a, "-L") == 0 || strcmp(a, "--landscape") == 0)
        {
            landscape = 1;
        }
        else
        {
            usage(argv[0]);
            return (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0)
                       ? EXIT_SUCCESS
                       : EXIT_FAILURE;
        }
    }

    if (max_bits < 2 || max_bits > MAX_BITS_LIMIT)
    {
        fprintf(stderr, "-b は2〜%dで指定してください\n", MAX_BITS_LIMIT);
        return EXIT_FAILURE;
    }
    if (min_bits < 2 || min_bits > max_bits)
    {
        fprintf(stderr, "-m は2〜%dで指定してください\n", max_bits);
        return EXIT_FAILURE;
    }

    const uint64_t limit = UINT64_C(1) << max_bits;
    fprintf(stderr, "# メモ化配列: 2^%d * %zuB = %.1fMiB\n", max_bits,
            sizeof(delay_t),
            (double)limit * sizeof(delay_t) / (1024.0 * 1024.0));

    delay_t *delay = malloc(limit * sizeof(delay_t));
    if (!delay)
    {
        fprintf(stderr, "メモリを確保できませんでした\n");
        return EXIT_FAILURE;
    }

    delay[1] = 0;
    const uint64_t tick = UINT64_C(1) << 24;
    for (uint64_t n = 2; n < limit; n++)
    {
        uint64_t x = n;
        uint32_t s = 0;
        /* n未満に落ちるまで辿る。そこから先は計算済み */
        while (x >= n)
        {
            x = (x & 1) ? 3 * x + 1 : x >> 1;
            s++;
        }
        delay[n] = (delay_t)(s + delay[x]);
        if ((n & (tick - 1)) == 0)
        {
            fprintf(stderr, "\r# delay計算中: %.1f%%",
                    100.0 * (double)n / (double)limit);
            fflush(stderr);
        }
    }
    if (limit > tick)
    {
        fprintf(stderr, "\r# delay計算完了       \n");
    }

    printf("bits\tdelay\tper_bit\tvalue");
    if (verbose)
    {
        printf("\ttrailing_ones\tall_ones_delay");
    }
    if (stats)
    {
        printf("\tmean\tsd\tsd_per_sqrt_bit\trecord_z");
    }
    printf("\tbinary\n");

    for (int b = min_bits; b <= max_bits; b++)
    {
        const uint64_t lo = UINT64_C(1) << (b - 1);
        const uint64_t hi = (b == max_bits) ? limit : (UINT64_C(1) << b);
        const uint64_t cnt = hi - lo;
        uint64_t best = lo;
        delay_t bd = 0;
        double sum = 0.0, sumsq = 0.0;
        for (uint64_t n = lo; n < hi; n++)
        {
            if (delay[n] > bd)
            {
                bd = delay[n];
                best = n;
            }
            if (stats)
            {
                const double v = delay[n];
                sum += v;
                sumsq += v * v;
            }
        }

        printf("%d\t%u\t%.2f\t%" PRIu64, b, bd, (double)bd / b, best);
        if (verbose)
        {
            printf("\t%d\t%u", trailing_ones(best),
                   delay[(UINT64_C(1) << b) - 1]);
        }
        if (stats)
        {
            const double mean = sum / (double)cnt;
            const double var = sumsq / (double)cnt - mean * mean;
            const double sd = var > 0.0 ? sqrt(var) : 0.0;
            printf("\t%.1f\t%.2f\t%.3f\t%.2f", mean, sd, sd / sqrt((double)b),
                   sd > 0.0 ? (bd - mean) / sd : 0.0);
        }
        putchar('\t');
        for (int i = b - 1; i >= 0; i--)
        {
            putchar((best >> i & 1) ? '1' : '0');
        }
        putchar('\n');
    }

    if (stats)
    {
        /* 最大bit数について「N個引いたときの最大値」を実測する。
           delayはdelay_tに収まるので、ソートせず度数分布から分位点を求める */
        const uint64_t lo = UINT64_C(1) << (max_bits - 1);
        const uint64_t cnt = limit - lo;
        const size_t hsize = (size_t)1 << (8 * sizeof(delay_t));
        uint64_t *hist = calloc(hsize, sizeof(uint64_t));
        if (!hist)
        {
            fprintf(stderr, "度数分布を確保できませんでした\n");
            free(delay);
            return EXIT_FAILURE;
        }
        double sum = 0.0, sumsq = 0.0;
        for (uint64_t n = lo; n < limit; n++)
        {
            const double v = delay[n];
            sum += v;
            sumsq += v * v;
            hist[delay[n]]++;
        }
        const double mean = sum / (double)cnt;
        const double var = sumsq / (double)cnt - mean * mean;
        const double sd = var > 0.0 ? sqrt(var) : 0.0;

        printf("\n# %dbit の全%" PRIu64 "個から N個引いたときの最大delay\n",
               max_bits, cnt);
        printf("trials\tmax_delay\tz\n");
        for (double N = 100.0; N <= (double)cnt; N *= 10.0)
        {
            /* 上位 cnt/N 個に入る境目の値 = 1-1/N 分位点 */
            uint64_t need = (uint64_t)((double)cnt / N);
            if (need == 0)
            {
                need = 1;
            }
            uint64_t acc = 0;
            size_t q = hsize - 1;
            while (q > 0 && acc + hist[q] < need)
            {
                acc += hist[q];
                q--;
            }
            printf("%.0f\t%zu\t%.2f\n", N, q,
                   sd > 0.0 ? ((double)q - mean) / sd : 0.0);
        }
        free(hist);
    }

    if (landscape)
    {
        print_landscape(delay, max_bits, UINT64_C(1) << (max_bits - 1), limit);
    }

    free(delay);
    return EXIT_SUCCESS;
}
