
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    unsigned char a[8] = { 0x00 };
    a[7] = 0xf0;
    for (size_t i = 0; i < 8; i++)
    {
        printf("%02x", a[i]);
    }
    printf("\n");

    unsigned long t = *(unsigned long *)a;
    printf("%016lx, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));

    t = htole64(*(unsigned long *)a);
    printf("%016lx, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));

    t = htobe64(*(unsigned long *)a);
    printf("%016lx, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));

    t = le64toh(*(unsigned long *)a);
    printf("%016lx, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));

    t = be64toh(*(unsigned long *)a);
    printf("%016lx, %d, %d\n", t, __builtin_ctzl(t), __builtin_clzl(t));
    return 0;
}
