
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

const char *roulette(const char **table, const size_t tablesize)
{
    uint64_t a = 0;
    ssize_t s = getrandom(&a, 6, 0);
    if (s != 6)
    {
        perror("getrandom");
        return NULL;
    }
    double b = tablesize * (le64toh(a) / (double)(1UL << 48));

    return table[(size_t)b];
}
