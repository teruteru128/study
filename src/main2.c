
#define _GNU_SOURCE
#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

int hiho(int argc, char **argv, const char **envp)
{
    const size_t size = 32UL;
    const size_t num = 16777216UL;
    const size_t length = size * num;
    unsigned char *buf = malloc(length);
    ssize_t ge = 0;
    char filename[PATH_MAX] = "";
    FILE *fout = NULL;
    for (size_t i = 8; i < 256; i++)
    {
        printf("(%zu)乱数取得中\n", i);
        ge = getrandom(buf, length, GRND_RANDOM);
        printf("(%zu)乱数取得完了\n", i);
        snprintf(filename, PATH_MAX, "privateKeys%zu.bin", i);
        fout = fopen(filename, "wb");
        if (fout == NULL)
        {
            perror("fopen wb");
            break;
        }
        fwrite(buf, size, num, fout);
        fclose(fout);
        printf("(%zu)乱数ファイル書き込み完了\n", i);
        fout = NULL;
    }

    memset(buf, 0, length);
    free(buf);
    return 0;
}
