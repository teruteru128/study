
#define _GNU_SOURCE
#include "config.h"

#include <complex.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hiho(int argc, char **argv, const char **envp)
{
    size_t count = 0;
    // under bust 65cm
    double ub = 62.0;
    char cup[12] = "";
    for (size_t offset = 0; offset < 11; offset++)
    {
        for (size_t alphabet = 0; alphabet < 26; alphabet++, count++)
        {
            cup[offset] = 'A' + alphabet;
            printf("%s, %zu, %6.1lfcm\n", cup, count, ub + 10 + 2.5 * count);
        }
    }

    printf("%zu\n", sizeof(float complex));
    return 0;
}
