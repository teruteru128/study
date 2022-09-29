
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

/**
 * @brief 乱数デバイスから乱数を読み込んで[0,
 * 1)な倍精度浮動小数点数の乱数を返す
 * 最初から52bitだけ読み込んで変換したほうが効率がいいのでそうすべき
 *
 * @return int
 */
int main(int argc, char **argv)
{
    int32_t seed;
    ssize_t numberOfRandomBytes
        = getrandom(&seed, sizeof(int32_t), GRND_NONBLOCK);
    if (numberOfRandomBytes < 0)
    {
        return 1;
    }
    srandom(seed);
    // 31bits
    unsigned long l = (unsigned long)random();
    // 62bits
    l = (l << 31) | (unsigned long)random();
    double r = (double)l / ((1UL << 62) - 1);
    printf("%.24lf\n", r);
    return EXIT_SUCCESS;
}
