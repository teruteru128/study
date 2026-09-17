/**
 * コラッツの上昇区間が長くなる初期値を生成する。
 *
 * 下位Kbitを1で埋めた数 n = (r+1)*2^K - 1 は、K回連続で「3n+1して1回だけ右シフト」
 * を繰り返す:
 *
 *   n_i = 3^i * (r+1) * 2^(K-i) - 1   (i = 0 .. K-1, いずれも奇数、ステップ2i)
 *
 * n_{K-1} = 2*3^(K-1)*(r+1) - 1 に 3n+1 を適用した 2*3^K*(r+1) - 2 が頂点で、
 * 次のシフトで 3^K*(r+1) - 1 という偶数に落ちて上昇が終わる。頂点はシフト前の
 * 値なので、
 *
 *   最大到達ステップ >= 2*K - 1
 *   最大到達bit数   >= 開始bit数 + 1 + K*log2(3/2)
 *
 * となり、Kを伸ばすぶんだけ上昇区間を長くできる。これはKbitの1並びが保証する
 * 下限で、上位(N-K)bitの引き次第では上昇がさらに続くこともある。下限値はstderr
 * に出すのでhugecollatzの実測と突き合わせられる。
 *
 *   stdout : 生成した初期値(16進、1行1個)
 *   stderr : 生成条件と上昇区間の予測値
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <time.h>

#include <gmp.h>

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [OPTIONS]\n"
            "  -k, --ones K      下位Kbitを1で埋める(上昇区間の長さを決める)\n"
            "  -b, --bits N      生成する数全体のbit数(既定: Kと同じ = 2^K-1)\n"
            "                    N > K のとき上位(N-K)bitは乱数で埋める\n"
            "  -c, --count C     生成する個数(既定: 1)\n"
            "  -s, --seed S      乱数シード(既定: /dev/urandom、値はstderrに出す)\n"
            "  -d, --dec         10進で出力する(既定: 16進)\n"
            "\n"
            "例: 下位1024bitが1、全体65536bitの初期値を3個\n"
            "    %s -k 1024 -b 65536 -c 3 | while read n; do hugecollatz \"$n\"; done\n",
            argv0, argv0);
}

int main(int argc, char **argv)
{
    unsigned long ones = 0;
    unsigned long bits = 0;
    unsigned long count = 1;
    unsigned long seed = 0;
    int have_seed = 0;
    int base = 16;

    for (int i = 1; i < argc; i++)
    {
        const char *a = argv[i];
        if ((strcmp(a, "-k") == 0 || strcmp(a, "--ones") == 0) && i + 1 < argc)
        {
            ones = strtoul(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-b") == 0 || strcmp(a, "--bits") == 0) &&
                 i + 1 < argc)
        {
            bits = strtoul(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-c") == 0 || strcmp(a, "--count") == 0) &&
                 i + 1 < argc)
        {
            count = strtoul(argv[++i], NULL, 10);
        }
        else if ((strcmp(a, "-s") == 0 || strcmp(a, "--seed") == 0) &&
                 i + 1 < argc)
        {
            seed = strtoul(argv[++i], NULL, 10);
            have_seed = 1;
        }
        else if (strcmp(a, "-d") == 0 || strcmp(a, "--dec") == 0)
        {
            base = 10;
        }
        else
        {
            usage(argv[0]);
            return (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0)
                       ? EXIT_SUCCESS
                       : EXIT_FAILURE;
        }
    }

    if (ones == 0)
    {
        fprintf(stderr, "-k は1以上を指定してください\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (bits == 0)
    {
        bits = ones;
    }
    if (bits < ones)
    {
        fprintf(stderr, "-b (%lu) は -k (%lu) 以上でなければなりません\n", bits,
                ones);
        return EXIT_FAILURE;
    }
    if (count == 0)
    {
        return EXIT_SUCCESS;
    }
    if (!have_seed)
    {
        if (getrandom(&seed, sizeof(seed), 0) != (ssize_t)sizeof(seed))
        {
            seed = (unsigned long)time(NULL);
        }
    }

    gmp_randstate_t rs;
    gmp_randinit_mt(rs);
    gmp_randseed_ui(rs, seed);

    /* 頂点は3n+1の直後(シフト前)なので、利得は1 + K*log2(3/2) */
    double gain = 1.0 + (double)ones * 0.5849625007211562;
    fprintf(stderr,
            "# bits=%lu ones=%lu count=%lu seed=%lu\n"
            "# 下限: max_step>=%lu max_bits>=%.0f (開始+%.0fbit)\n",
            bits, ones, count, seed, 2UL * ones - 1, (double)bits + gain,
            gain);

    mpz_t n, mask;
    mpz_init(n);
    mpz_init(mask);
    /* mask = 2^ones - 1 */
    mpz_set_ui(mask, 1);
    mpz_mul_2exp(mask, mask, ones);
    mpz_sub_ui(mask, mask, 1);

    for (unsigned long c = 0; c < count; c++)
    {
        if (bits > ones)
        {
            /* 上位(bits-ones)bitを乱数で埋め、全体がちょうどbits bitになるよう
               最上位bitを立てる */
            mpz_urandomb(n, rs, bits - ones);
            mpz_setbit(n, bits - ones - 1);
            mpz_mul_2exp(n, n, ones);
            mpz_ior(n, n, mask);
        }
        else
        {
            mpz_set(n, mask); /* 2^ones - 1 */
        }
        mpz_out_str(stdout, base, n);
        fputc('\n', stdout);
    }

    mpz_clear(n);
    mpz_clear(mask);
    gmp_randclear(rs);
    return EXIT_SUCCESS;
}
