
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    unsigned char a[8] = { 0x00 };
    if (argc == 2)
    {
        a[7] = strtol(argv[1], NULL, 0);
    }
    else if (argc >= 3)
    {
        if (strcmp(argv[1], "long") == 0)
        {
            *(unsigned long *)a = be64toh(strtoul(argv[2], NULL, 0));
        }
        else
        {
            a[7] = strtol(argv[1], NULL, 0);
        }
    }
    else
    {
        a[7] = 0xf0;
    }
    for (size_t i = 0; i < 8; i++)
    {
        printf("%02x", a[i]);
    }
    printf(", %016lx\n", *(unsigned long *)a);
    if (*(unsigned long *)a == 0)
    {
        fprintf(stderr, "Zero is undefined and terminates.\n");
        return 1;
    }

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
