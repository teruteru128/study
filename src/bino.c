
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
    for (i = 0; i <= X; i++)
    {
        for (j = 0; j <= i; j++)
        {
            if (j != 0)
            {
                printf(" ");
            }
            printf("%" PRIu64, binomial(i, j));
        }
        printf("\n");
    }
    return EXIT_SUCCESS;
}
