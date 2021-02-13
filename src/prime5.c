
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

/**
 * @brief 素数探索の初期値を生成
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    mpz_t a;
    mpz_inits(a, NULL);
    unsigned char p[32768];
    FILE *fin = fopen("/dev/urandom", "rb");
    fread(p, 1, 32768, fin);
    fclose(fin);
    mpz_import(a, 32768, 1, sizeof(char), 0, 0, p);

    mpz_setbit(a, 262143);
    mpz_clrbit(a, 0);
    mpz_out_str(stdout, 16, a);
    fputs("\n", stdout);
    mpz_clears(a, NULL);
    return 0;
}
