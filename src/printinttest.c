
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <printint.h>

#define OUT_SIZE 21

struct testcase
{
    uint64_t in;
    const char *p;
};

int main(int argc, char **argv)
{
    struct testcase testcase[] = {{0UL, "0"},
                                  {1UL, "1"},
                                  {10UL, "10"},
                                  {100UL, "100"},
                                  {1000UL, "1000"},
                                  {10000UL, "10000"},
                                  {100000UL, "100000"},
                                  {1000000UL, "1000000"},
                                  {10000000UL, "10000000"},
                                  {100000000UL, "100000000"},
                                  {1000000000UL, "1000000000"},
                                  {10000000000UL, "10000000000"},
                                  {100000000000UL, "100000000000"},
                                  {1000000000000UL, "1000000000000"},
                                  {10000000000000UL, "10000000000000"},
                                  {100000000000000UL, "100000000000000"},
                                  {1000000000000000UL, "1000000000000000"},
                                  {10000000000000000UL, "10000000000000000"},
                                  {100000000000000000UL, "100000000000000000"},
                                  {1000000000000000000UL, "1000000000000000000"},
                                  {10000000000000000000UL, "10000000000000000000"}};
    char out1[OUT_SIZE];
    memset(out1, 0, OUT_SIZE);

    for (size_t i = 0; i < 21; i++)
    {
        snprintUInt64(out1, OUT_SIZE, testcase[i].in);
        if (strcmp(out1, testcase[i].p) != 0)
        {
            fprintf(stdout, "NG : %lu\n", testcase[i].in);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
