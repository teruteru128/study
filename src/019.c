
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
y^2 + (x - 0.5)^2 = 1
x^2 - x + y^2 = 0
x^2 - x + 0.25 - 0.25+ y^2 = 0
(x-0.5)^2 + y^2 = 0.25
--
y^2 + (x-1)^2 = 1
y^2 = 1 - (x^2 -2x +1)
y^2 = 2x - x^2
y = sqrt(2x - x^2)
y = sqrt((2 - x)x)
--
y = 2 sqrt(x - x^2)
  = 2 sqrt((1 - x) x)
*/
int main(int argc, char **argv)
{
    double a = 0x1.14514p-1;
    printf("%.18lf\n", a);
    while(0 < a && a < 1)
    {
        a = 2 * sqrt((1 - a) * a);
        printf("%.18lf\n", a);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
