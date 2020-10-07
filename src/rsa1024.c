
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#define N "c05748bbfb5acd7e" \
          "5a77dc03d9ec7d8b" \
          "b957c1b95d9b2060" \
          "90d83fd1b67433ce" \
          "83ead7376ccfd612" \
          "c72901f4ce0a2e07" \
          "e322d438ea4f3464" \
          "7555d62d04140e10" \
          "84e999bb4cd5f947" \
          "a76674009e231854" \
          "9fd102c5f7596edc" \
          "332a0ddee3a35518" \
          "6b9a046f0f96a279" \
          "c1448a9151549dc6" \
          "63da8a6e89cf8f51" \
          "1baed6450da2c1cb"
#define BASE 16

/*
 * 135066410865995223349603216278805969938881475605667027524485143851526510604859533833940287150571909441798207282164471551373680419703964191743046496589274256239341020864383202110372958725762358509643110564073501508187510676594629205563685529475213500852879416377328533906109750544334999811150056977236890927563
 * RSA-1024-challenge
 */
int main(int argc, char *argv[])
{
    size_t i = 0;
    mpz_t n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num;
    mpz_t t[16];
    mpz_inits(n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num, NULL);
    for (i = 0; i < 16; i++)
    {
        mpz_init(t[i]);
    }
    mpz_set_str(n, N, BASE);
    // qを1からsqrt(n)まで*2ずつ, (n-q^2)/qがqより大きくなるまでループ
    // for(q = 1; (n-q^2)/q < q; q *= 2)
    mpz_set_str(p, "6101033566922210362775848843043422665924052993891242451932861862025497930636586185033910893703459369951328434771914583346105161468964915448455746310700313", 10);
    mpz_mul_ui(q, p, 2);
    mpz_mod(r, n, p);
    gmp_printf("%Zd\n", r);
    mpz_mod(num, n, q);
    gmp_printf("%Zd\n", num);
    int cmp = mpz_cmp(r, num);
    printf("r %s num\n", cmp == 0 ? "=" : (cmp > 0 ? ">": "<"));
    for(mpz_set_si(q, 1), mpz_root(maxQ, n, 2); mpz_cmp(q, maxQ) < 0; mpz_mul_si(q, q, 2))
    {
        mpz_pow_ui(r, q, 2);
        mpz_sub(r, n, r);
        mpz_tdiv_qr(r, num, r, q);
        gmp_printf("%Zd, %Zd\n", r, num);
    }

    mpz_clears(n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num, NULL);
    for (i = 0; i < 16; i++)
    {
        mpz_clear(t[i]);
    }
    return 0;
}
