
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    long double a = logl(1e300L);
    long double b = logl(2);
    long double c = logl(10);
    long double d = logl(16);
    printf("%28.24Lf\n", a);
    printf("%28.24Lf\n", b);
    printf("%28.24Lf\n", c);
    printf("%28.24Lf\n", d);
    printf("%.24Lf\n", a / b);
    printf("%.24Lf\n", a / c);
    printf("%.24Lf\n", a / d);
#ifdef _GNU_SOURCE
    printf("%.24Lf\n", powl(10, M_El));
#endif
    long double e = LDBL_EPSILON;
    printf("%La\n", e);
    printf("%Le\n", e);
    char format[32] = "";
    printf("%.63Lf\n", e);
    printf("%Lg\n", e);
    return EXIT_SUCCESS;
}
