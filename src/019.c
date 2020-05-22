
#include <stdio.h>
#include <math.h>

int main(int argc, char **argv)
{
#if 1
    double a = 0.114514;
    printf("%.18lf\n", a);
    while (a > 0)
    {
        a = 2 * sqrt(a - pow(a, 2));
        printf("%.18lf\n", a);
    }
    printf("\n");
#endif
    int i = 0;
    if(!i){
        printf("true\n");
    }else{
        printf("false\n");
    }
    return 0;
}
