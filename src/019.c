
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv)
{
    double a = 0.114514;
    printf("%.18lf\n", a);
    while (a > 0)
    {
        a = 2 * sqrt(a - pow(a, 2));
        printf("%.18lf\n", a);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
