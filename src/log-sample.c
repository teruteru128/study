
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

int main(int argc, char **argv)
{
    long double a = 1e+300L;
    a = logl(a);
    long double b = logl(2);
    printf("%.24Lf\n", a);
    printf("%.24Lf\n", b);
    printf("%.24Lf\n", a / b);
    printf("%.24Lf\n", powl(10, a / b));
    return EXIT_SUCCESS;
}
