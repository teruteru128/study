#include <stdio.h>

int main(void)
{
    double s = 0;
    double r = 0;
    double t;
    unsigned long x = 1;
    while (s < 30)
    {
        t = s;
        r += (1.0 / (double)x);
        x++;
        s += r;
        t -= s;
        r += t;
        if ((x % 100000000) == 0)
            printf("%ld, %lf\n", x, s);
    }
    printf("%ld, %.60f\n", x, s);

    return 0;
}
