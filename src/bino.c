
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "biom.h"
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gmp.h>

#define X 25

/**
 * binomial coefficient
 * 二項係数プログラム
 * binomial coefficient
 *
 * bino
 * 二項係数のピラミッドが1から25まで並ぶ
 *
 * bino 13
 * 二項係数の13段目のみ列挙
 *
 * bino 13 6
 * 二項係数の13の6のみ
 */
int main(int argc, char *argv[])
{

    int i, j;
    mpz_t k, l, r;
    mpz_init(r);
    for (i = 0; i <= X; i++)
    {
        for (j = 0; j <= i; j++)
        {
            if (j != 0)
            {
                printf(" ");
            }
            mpz_bin_ui(r, NULL, 0);
            mpz_bin_uiui(r, i, j);
            printf("%" PRIu64, binomial(i, j));
        }
        printf("\n");
    }
    mpz_clear(r);
    return EXIT_SUCCESS;
}
