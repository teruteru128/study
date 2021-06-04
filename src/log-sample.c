
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char **argv)
{
    long double a = 1e+300L;
    a = logl(a);
    long double b = logl(2);
    printf("%.24Lf\n", a);
    printf("%.24Lf\n", b);
    printf("%.24Lf\n", a / b);
#ifdef _GNU_SOURCE
    printf("%.24Lf\n", powl(10, M_El));
#endif
    long double c = LDBL_EPSILON;
    printf("%La\n", c);
    syslog(LOG_NOTICE, "HELLO WORLD");
    return EXIT_SUCCESS;
}
