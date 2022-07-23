
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>

int hiho(int argc, char **argv)
{
    if (argc > 1)
        printf("%c\n", argv[1][0] + 8);
    else
        printf("%c\n", 'G' + 8);
    return 0;
}
