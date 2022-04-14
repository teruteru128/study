
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s <num>\n", argv[0]);
        return 1;
    }

    uint64_t n = strtoul(argv[1], NULL, 10);
    size_t count = 0;
    size_t count_triple = 0;

    printf("%zu, %ld\n", count + 1, n);
    while (n > 1)
    {
        if (n & 1)
        {
            n = n * 3 + 1;
            count_triple++;
        }
        else
        {
            n /= 2;
        }
        count++;
        printf("%zu, %zu, %ld\n", count + 1, count_triple, n);
    }

    return 0;
}
