
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static void ma1(uint64_t a)
{
    unsigned long base10;
    unsigned long base16;
    base10 = (unsigned long)log10(a);
    base16 = (unsigned long)(log2(a) / 4);
    printf("%lu - %lu = %lu\n", base10, base16, base10 - base16);
}

static void ma2(uint64_t a)
{
    char buf[22] = "";
    size_t length1 = 0;
    size_t length2 = 0;
    length1 = snprintf(buf, 22, "%lu", a) - 1;
    length2 = snprintf(buf, 22, "%lx", a) - 1;
    printf("%lu - %lu = %lu\n", length1, length2, length1 - length2);
}

/**
 * @brief
 * 10進数表記と16進数表記で2桁違うのはいくらかいくらまでなんやっちゅう話や
 * solve 2 <= log(10, x) - log(16, x) , log(10, x) - log(16, x) < 3
 * 1000000 <= x < 1048576
 * 桁数の差を求める
 * find the difference in the number of digits
 * @return int
 */
int fu()
{
    ma1(999999);
    ma1(1000000);
    ma1(0xfffff);
    ma1(0x100000);

    ma1(9999999);
    ma1(10000000);
    ma1(0xffffff);
    ma1(0x1000000);

    ma1(99999999);
    ma1(100000000);
    ma1(0xfffffff);
    ma1(0x10000000);

    ma1(999999999);
    ma1(1000000000);
    ma1(0xffffffffUL);
    ma1(0x100000000UL);

    ma1(9999999999UL);
    ma1(10000000000UL);
    ma1(0xfffffffffUL);
    ma1(0x1000000000UL);

    ma1(99999999999UL);
    ma1(100000000000UL);
    // ここまで16進数の桁数に10進数の桁数に差を詰める
    // log(10, 628288020076) ≒ log(16, 628288020076)
    printf("!!\n");
    // ここは10進数の桁数が差を開く
    ma1(999999999999UL);
    ma1(1000000000000UL);

    // ここから10進数の桁数が16進数の桁数に差を開く
    ma1(0xffffffffffUL);
    ma1(0x10000000000UL);
    ma1(10000000000000UL);
    ma1(10000000000000UL);

    ma1(0xfffffffffffUL);
    ma1(0x100000000000UL);
    ma1(99999999999999UL);
    ma1(100000000000000UL);

    ma1(0xffffffffffffUL);
    ma1(0x1000000000000UL);
    ma1(999999999999999UL);
    ma1(1000000000000000UL);

    ma1(0xfffffffffffffUL);
    ma1(0x10000000000000UL);
    ma1(9999999999999999UL);
    ma1(10000000000000000UL);

    ma2(0xffffffffffffffUL);
    ma2(0x100000000000000UL);
    ma2(99999999999999999UL);
    ma2(100000000000000000UL);

    return 0;
}
