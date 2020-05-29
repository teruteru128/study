
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv)
{
    double a = log(1 << 18);
    double b = log(10);
    printf("%.24lf\n", a / b);
    printf("%.24lf\n", pow(10, a / b));
    return EXIT_SUCCESS;
}
