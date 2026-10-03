/* mr2fs_strong_base2 を、mpz_powm による標準的な強い確率的素数判定と突き合わせる。 */
#include "mr2fs.h"

#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

/* 基準実装: n が底2の強い確率的素数か */
static int ref_strong_base2(mpz_srcptr n)
{
    mpz_t nm1, d, y, two;
    mpz_inits(nm1, d, y, two, NULL);
    mpz_sub_ui(nm1, n, 1);
    mp_bitcnt_t s = mpz_scan1(nm1, 0);
    mpz_fdiv_q_2exp(d, nm1, s);
    mpz_set_ui(two, 2);
    mpz_powm(y, two, d, n);
    int ok = !mpz_cmp_ui(y, 1) || !mpz_cmp(y, nm1);
    for (mp_bitcnt_t i = 1; i < s && !ok; i++) {
        mpz_powm_ui(y, y, 2, n);
        if (!mpz_cmp(y, nm1))
            ok = 1;
    }
    mpz_clears(nm1, d, y, two, NULL);
    return ok;
}

static int bad, total, passed;

static void check(mpz_srcptr n, const char *what)
{
    int got = mr2fs_strong_base2(n);
    if (got < 0) {
        printf("UNSUPPORTED (%s, %lu bits)\n", what, (unsigned long)mpz_sizeinbase(n, 2));
        bad++;
        return;
    }
    int ref = ref_strong_base2(n);
    total++;
    passed += ref;
    if (got != ref) {
        printf("MISMATCH (%s, %lu bits): got %d, ref %d\n", what, (unsigned long)mpz_sizeinbase(n, 2),
               got, ref);
        bad++;
    }
}

int main(void)
{
    if (!mr2fs_available()) {
        printf("AVX2/FMA が使えないためスキップ\n");
        return 0;
    }
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, 20261002);
    mpz_t n, k;
    mpz_inits(n, k, NULL);

    /* ランダムな奇数(ほぼ全部合成数)と、nextprime による素数 */
    unsigned sizes[] = {16, 17, 31, 64, 100, 257, 600};
    for (size_t si = 0; si < sizeof sizes / sizeof *sizes; si++) {
        unsigned limbs = sizes[si];
        for (int t = 0; t < 8; t++) {
            mpz_urandomb(n, rs, 64UL * limbs);
            mpz_setbit(n, 64UL * limbs - 1);
            mpz_setbit(n, 0);
            check(n, "random odd");
        }
        /* mpz_nextprime は大きいサイズだと遅いので、素数は小さいサイズだけ */
        for (int t = 0; t < (limbs <= 100 ? 3 : 0); t++) {
            mpz_urandomb(n, rs, 64UL * limbs);
            mpz_setbit(n, 64UL * limbs - 1);
            mpz_nextprime(n, n);
            if (mpz_sizeinbase(n, 2) == 64UL * limbs)
                check(n, "prime");
        }
    }
    /* m-1 = 2^s * odd で s が大きい数(s のループとリム境界をまたぐ s を踏ませる) */
    unsigned limbs = 20;
    for (unsigned s = 1; s <= 64UL * limbs - 70; s += (s < 140 ? 1 : 37)) {
        for (int t = 0; t < 3; t++) {
            mpz_urandomb(k, rs, 64UL * limbs - s);
            mpz_setbit(k, 64UL * limbs - s - 1);
            mpz_setbit(k, 0);
            mpz_mul_2exp(n, k, s);
            mpz_add_ui(n, n, 1);
            check(n, "proth-like");
        }
    }
    printf("checked %d numbers (%d strong probable primes)\n", total, passed);
    printf(bad ? "FAILED: %d\n" : "ALL OK\n", bad);
    return bad != 0;
}
