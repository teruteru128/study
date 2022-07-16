
#define _GNU_SOURCE
#include "config.h"

#include <complex.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <sys/random.h>

int main(int argc, char const *argv[])
{
    // 2の累乗の中で最も10の累乗に近いのは？
    // n*log(2)/log(10)
    long double a = 10;
    long double b, c;
    long double min = 1;
    long double max = 0;
    for (long i = 1; i <= 20000; i++)
    {
        a = i * log10l(2);
        b = modfl(a, &c);
        if (b < min)
        {
            min = b;
            printf("min: %15ld, %.20Lf, %6ld\n", i, b, (long)c);
        }
        if (b > max)
        {
            max = b;
            printf("max: %15ld, %.20Lf, %6ld\n", i, b, (long)c);
        }
    }
    return 0;
}
