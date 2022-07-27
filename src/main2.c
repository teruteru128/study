
#define _GNU_SOURCE
#include "config.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *hex2bin(const char *hex, unsigned char *bin, size_t binsize)
{
    if (hex && bin)
    {
        for (; binsize && isxdigit(hex[0]) && isxdigit(hex[1]);
             hex += 2, bin += 1, binsize -= 1)
        { //  fujitanozomu さんの指摘を適用
            int r = sscanf(hex, "%2hhx", bin); //  shiracamus さんの指摘を適用
            if (r != 1)
            {
                break;
            }
        }
    }

    return hex;
}

int hiho(int argc, char **argv, const char **envp)
{
    size_t lines = 0;
    FILE *fin = NULL;
    char pathname[PATH_MAX];
    unsigned char pubkeybuf[65] = { 0 };
    hex2bin(
        "0400000063ca291bb91c80a6141de191592c8f5a50b522b523fbb1cdf5814ea586dfc"
        "6aa167418f1eea92621f4f4544c591f9ee8334b7ac1dcc27917f1edf993aa",
        pubkeybuf, 65);

    unsigned char *buf = malloc(16777216 * 65);
    unsigned char *catch = NULL;
    for (size_t i = 0; i < 8; i++)
    {
        snprintf(pathname, PATH_MAX, "publicKeys%zu.bin", i);
        fin = fopen(pathname, "rb");
        if (fin == NULL)
        {
            perror("fopen");
            break;
        }
        if (fread(buf, 65, 16777216, fin) != 16777216)
        {
            perror("fread");
            fclose(fin);
            exit(EXIT_FAILURE);
        }
        if ((catch = memmem(buf, 16777216 * 65, pubkeybuf, 65)) != NULL)
        {
            printf("%s, %ld\n", pathname, (catch - buf) / 65);
        }
        fclose(fin);
    }
    memset(buf, 0, 16777216 * 65);
    free(buf);

    return 0;
}
