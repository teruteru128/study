
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#define X 25

// TODO オーバーフロー対策
uint64_t binomial(uint64_t n, uint64_t k)
{
    // 分母
    uint64_t denominator = 1;
    // 分子
    uint64_t numerator = 1;
    k = MIN(k, n - k);
    size_t i, j, l = n - k + 1;
    for (j = n; j >= l; j--)
    {
        numerator *= j;
    }
    for (i = k; i >= 1; i--)
    {
        denominator *= i;
    }
    return numerator / denominator;
}

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
