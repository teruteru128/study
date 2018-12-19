
#include <stdio.h>

int main(int argc, char **argv)
{
    double a = 3.34;
    printf("%0.5lf\n", a);
    while (a > 0)
    {
        a *= 2;
        if (a >= 1)
        {
            printf("1");
            a -= 1;
        }
        else
        {
            printf("0");
        }
    }
    printf("\n");
    return 0;
}
