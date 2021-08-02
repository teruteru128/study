
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    unsigned char a[8] = { 0x00 };
    a[7] = 0xf0;
    unsigned long t = *(unsigned long *)a;
    printf("%lu, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));
    t = htole64(*(unsigned long *)a);
    printf("%lu, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));
    t = htobe64(*(unsigned long *)a);
    printf("%lu, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));
    return 0;
}
