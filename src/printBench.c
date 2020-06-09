
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "printint.h"

#define BUF_SIZE 21
#define LOOP_COUNT 100000000ULL

/**
 * printint benchmark
 * */
int main(int argc, char **argv)
{
    char buf[BUF_SIZE];
    uint64_t t = 10000000000000000000ULL;
    size_t i;
    size_t len;
    for (i = 0; i < LOOP_COUNT; i++)
    {
        len = snprintUInt64(buf, BUF_SIZE, t);
        //snprintf(buf, BUF_SIZE, "%" PRIu64, t);
        //len = strlen(buf);
    }
    printf("%" PRIu64 "\n", len);
}
