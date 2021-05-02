
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <inttypes.h>

/**
 * @brief 
 * 
 * シーケンス
 * https://twitter.com/teruteru128/status/1108266625352892416
 * seq 最大
 * seq 最小 最大
 * seq 最小 個数
 * seq 最小 間隔 個数
 */
int main(int argc, char *argv[])
{
    long min = 0;
    long max = 16;

    if (argc == 2)
    {
        max = strtol(argv[1], NULL, 10);
    }
    else if (argc == 3)
    {
        min = strtol(argv[1], NULL, 10);
        max = strtol(argv[2], NULL, 10);
    }

    for (long i = min; i < max; i++)
    {
        printf("%ld\n", i);
    }

    return EXIT_SUCCESS;
}
