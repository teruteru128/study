/* 篩の延長(2^38+1 〜 10^14 の素数)で必要な「E mod p」の取り方を実測する。
 *   候補は E + step*2 + 1 (E は 2,097,152bit の偶数、step は最大約671万)。
 *   p が step の範囲より十分大きければ、素数 p ごとに E mod p を1回求めれば全 step を判定できる。
 *
 * 比較するもの:
 *   direct: 素数ごとに mpz_fdiv_ui(E, p)
 *   tree  : 素数 m 個の積の木を作り、E mod 根 から剰余の木で降ろす
 *           (積の木は偶数が複数あっても1回で済むので build と per-even を分けて表示する)
 *
 * 使い方: esieve_bench [m=32768] [leaf=32] [start=1e13] [bits=2097152]
 *   m と m/leaf は2のべき乗にすること。
 * ビルド: gcc -O3 -march=native -o esieve_bench esieve_bench.c -lgmp
 */
#include <gmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STEPMAX 6710886UL

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* E mod p が e のとき、E + 2*step + 1 ≡ 0 (mod p) となる最小の step (0 <= step < p) */
static uint64_t hit_step(uint64_t e, uint64_t p)
{
    uint64_t t = (e + 1) % p;
    t = (p - t) % p;
    return (uint64_t)(((unsigned __int128)t * ((p + 1) / 2)) % p);
}

int main(int argc, char **argv)
{
    size_t m = argc > 1 ? strtoul(argv[1], 0, 10) : 32768;
    size_t leaf = argc > 2 ? strtoul(argv[2], 0, 10) : 32;
    uint64_t start = argc > 3 ? (uint64_t)strtod(argv[3], 0) : 10000000000000ULL;
    size_t bits = argc > 4 ? strtoul(argv[4], 0, 10) : 2097152;
    size_t nleaf = m / leaf;
    if (m % leaf || (nleaf & (nleaf - 1))) {
        fprintf(stderr, "m は leaf の倍数で、m/leaf は2のべき乗にすること\n");
        return 1;
    }

    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, 12345);
    mpz_t E, t;
    mpz_inits(E, t, NULL);
    mpz_urandomb(E, rs, bits);
    mpz_setbit(E, bits - 1);
    mpz_clrbit(E, 0);

    uint64_t *pr = malloc(m * sizeof *pr);
    mpz_set_ui(t, start);
    for (size_t i = 0; i < m; i++) {
        mpz_nextprime(t, t);
        pr[i] = mpz_get_ui(t);
    }
    printf("E: %zu bit, 素数 %zu 個 (%lu 〜 %lu), leaf=%zu\n", bits, m, (unsigned long)pr[0],
           (unsigned long)pr[m - 1], leaf);

    /* ---- direct ---- */
    uint64_t *ed = malloc(m * sizeof *ed);
    double t0 = now();
    for (size_t i = 0; i < m; i++)
        ed[i] = mpz_fdiv_ui(E, pr[i]);
    double t_direct = now() - t0;
    printf("direct : %.3f s  = %.2f us/素数\n", t_direct, t_direct / m * 1e6);

    /* ---- 積の木 ---- */
    size_t nlev = 1;
    for (size_t n = nleaf; n > 1; n >>= 1) nlev++;
    mpz_t **P = malloc(nlev * sizeof *P), **R = malloc(nlev * sizeof *R);
    size_t *cnt = malloc(nlev * sizeof *cnt);
    t0 = now();
    cnt[0] = nleaf;
    for (size_t l = 0; l < nlev; l++) {
        if (l) cnt[l] = cnt[l - 1] / 2;
        P[l] = malloc(cnt[l] * sizeof(mpz_t));
        R[l] = malloc(cnt[l] * sizeof(mpz_t));
        for (size_t i = 0; i < cnt[l]; i++) { mpz_init(P[l][i]); mpz_init(R[l][i]); }
    }
    for (size_t g = 0; g < nleaf; g++) {
        mpz_set_ui(P[0][g], 1);
        for (size_t j = 0; j < leaf; j++) mpz_mul_ui(P[0][g], P[0][g], pr[g * leaf + j]);
    }
    for (size_t l = 1; l < nlev; l++)
        for (size_t i = 0; i < cnt[l]; i++) mpz_mul(P[l][i], P[l - 1][2 * i], P[l - 1][2 * i + 1]);
    double t_build = now() - t0;
    printf("tree   : 積の木 %.3f s (根 %zu bit) = %.2f us/素数 (偶数が複数でも1回)\n", t_build,
           mpz_sizeinbase(P[nlev - 1][0], 2), t_build / m * 1e6);

    /* ---- 剰余の木 (1つの偶数ぶん) ---- */
    t0 = now();
    mpz_tdiv_r(R[nlev - 1][0], E, P[nlev - 1][0]);
    double t_top = now() - t0;
    t0 = now();
    for (size_t l = nlev - 1; l-- > 0;)
        for (size_t i = 0; i < cnt[l]; i++) mpz_tdiv_r(R[l][i], R[l + 1][i / 2], P[l][i]);
    double t_desc = now() - t0;
    t0 = now();
    size_t mism = 0, hits = 0;
    for (size_t g = 0; g < nleaf; g++)
        for (size_t j = 0; j < leaf; j++) {
            size_t i = g * leaf + j;
            uint64_t e = mpz_fdiv_ui(R[0][g], pr[i]);
            if (e != ed[i]) mism++;
            if (hit_step(e, pr[i]) < STEPMAX) hits++;
        }
    double t_leaf = now() - t0;
    double per_even = t_top + t_desc + t_leaf;
    printf("tree   : E mod 根 %.3f s, 降下 %.3f s, 葉 %.3f s => 1偶数 %.3f s = %.2f us/素数\n", t_top,
           t_desc, t_leaf, per_even, per_even / m * 1e6);
    printf("検算   : directとの不一致 %zu 件, step<%lu となる素数 %zu 個 (期待値 約%.3f)\n", mism, STEPMAX, hits,
           (double)m * STEPMAX / (double)pr[m / 2]);
    printf("比     : direct / tree(1偶数) = %.2f, direct / tree(build込み, 偶数2本で按分) = %.2f\n",
           t_direct / per_even, t_direct / (per_even + t_build / 2));
    return mism ? 2 : 0;
}
