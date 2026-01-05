
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s <num>\n", argv[0]);
        return 1;
    }

    uint64_t n = strtoul(argv[1], NULL, 10);
    size_t count = 0;
    char buf[16];

    printf("%3zu,      , %ld\n", count + 1, n);
    while (n > 1)
    {
        if (n & 1)
        {
            n = n * 3 + 1;
            memcpy(buf, "*3+1", 5);
        }
        else
        {
            n /= 2;
            memcpy(buf, "/2", 3);
        }
        count++;
        printf("%3zu, %5s, %ld\n", count + 1, buf, n);
    }

    return 0;
}
