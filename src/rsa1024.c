
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
 */
#define qfile "q.txt"

int loadP(mpz_t *p)
{
    char buf[1024];
    FILE *fp = NULL;
    int ret = EXIT_SUCCESS;
    char *tmp = NULL;
    if ((fp = fopen(qfile, "r")) != NULL)
    {
        tmp = fgets(buf, 1024, fp);
        if (!tmp)
        {
            return EXIT_FAILURE;
        }
        mpz_set_str(*p, buf, 16);
    }
    else
    {
        perror("fopen");
        mpz_set_si(*p, 1);
        ret = EXIT_FAILURE;
    }
    fclose(fp);
    return ret;
}

/*
 * RSA-1024-challenge
 */
int main(int argc, char *argv[])
{
    size_t i = 0;
    mpz_t n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num;
    mpz_t t[16];
    mpz_inits(n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num, NULL);
    for(i = 0; i < 16; i++)
    {
        mpz_init(t[i]);
    }
    mpz_set_str(n, N, BASE);
    loadP(&q);
    // qを1からsqrt(n)までループ, (n-q^2)/qがqより大きくなるまでループ
    mpz_set_str(r, "b0000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002", 16);
    mpz_sqrt(sqrtN, n);
    mpz_mul(sqQ, q, q);
    mpz_mul_si(doubledSqQ, sqQ, 2);
    mpz_sub(nSubSqQ, n, sqQ);
    mpz_fdiv_qr(pSubQ, nSubSqQModQ, nSubSqQ, q);
    mpz_add(p, pSubQ, q);
    gmp_printf("n =              %Zd\n", n);
    gmp_printf("n =              %Zx\n", n);
    gmp_printf("2q^2 =           %Zd\n", doubledSqQ);
    gmp_printf("q =              %155Zd\n", q);
    gmp_printf("q =              %Zx\n", q);
    gmp_printf("sqrt(n) =        %155Zd\n", sqrtN);
    gmp_printf("n - q*q =        %Zd\n", nSubSqQ);
    gmp_printf("p - q =          %155Zd\n", pSubQ);
    gmp_printf("(n - q*q) %% q = %156Zd\n", nSubSqQModQ);
    mpz_clears(n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num, NULL);
    for(i = 0; i < 16; i++)
    {
        mpz_clear(t[i]);
    }
}
