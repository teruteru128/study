/* 本番サイズの残余を mpz_powm と丸ごと比較する検証プログラム。使い方: mr2fs_full <limbs> <seed> [fs|gmp|both]
   1件あたり fs は約3.4時間、gmp は約7時間かかるので、fs と gmp は別プロセスで走らせ、
   出力(残余の SHA-256 代わりの下位/上位リムの要約)を突き合わせる。 */
#include "mr2fs.h"

#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void summary(mpz_srcptr x)
{
    /* 全リムを混ぜた64bitの要約 + 先頭/末尾リム */
    mp_size_t n = mpz_size(x);
    mp_srcptr p = mpz_limbs_read(x);
    unsigned long h = 1469598103934665603UL;
    for (mp_size_t i = 0; i < n; i++)
        h = (h ^ p[i]) * 1099511628211UL, h ^= h >> 29;
    printf("residue hash %016lx low %016lx high %016lx\n", h, p[0], p[n - 1]);
}

int main(int argc, char **argv)
{
    mp_size_t limbs = argc > 1 ? atol(argv[1]) : 32768;
    unsigned long seed = argc > 2 ? strtoul(argv[2], 0, 10) : 1;
    const char *mode = argc > 3 ? argv[3] : "fs";
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, seed);
    mpz_t n, nm1, d, y, two, R, x;
    mpz_inits(n, nm1, d, y, two, R, x, NULL);
    mpz_urandomb(n, rs, 64UL * limbs);
    mpz_setbit(n, 64UL * limbs - 1);
    mpz_setbit(n, 0);
    time_t t0 = time(NULL);
    if (!strcmp(mode, "fs")) {
        mp_ptr out = malloc(limbs * 8);
        if (mr2fs_pow2d_mont(n, out) < 0) { puts("unsupported"); return 2; }
        mpz_import(x, limbs, -1, 8, 0, 0, out);
        puts("mode fs");
        summary(x);
    } else {
        mpz_sub_ui(nm1, n, 1);
        mp_bitcnt_t s = mpz_scan1(nm1, 0);
        mpz_fdiv_q_2exp(d, nm1, s);
        mpz_set_ui(two, 2);
        mpz_powm(y, two, d, n);
        mpz_setbit(R, 64UL * limbs);
        mpz_mul(y, y, R);
        mpz_mod(y, y, n);
        puts("mode gmp");
        summary(y);
    }
    printf("elapsed %ld s\n", (long)(time(NULL) - t0));
    return 0;
}
