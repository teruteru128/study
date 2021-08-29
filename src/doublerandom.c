
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief 乱数デバイスから乱数を読み込んで[0, 1)な倍精度浮動小数点数の乱数を返す
 * 最初から52bitだけ読み込んで変換したほうが効率がいいのでそうすべき
 * 
 * @return int 
 */
int main(argc, argv) int argc;
char **argv;
{
    FILE *in = fopen("/dev/urandom", "rb");
    if (!in)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
    unsigned int seed;
    size_t k = fread(&seed, sizeof(unsigned int), 1, in);
    if (k != 1)
    {
        perror("fopen");
        fclose(in);
        return EXIT_FAILURE;
    }
    fclose(in);
    srandom(seed);
    // 31bits
    unsigned long l = (unsigned long)random();
    // 62bits
    l = (l << 31) | (unsigned long)random();
    double r = (double)l / ((1UL << 62) - 1);
    printf("%.24lf\n", r);
    return EXIT_SUCCESS;
}
