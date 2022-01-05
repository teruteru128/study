
#define _GNU_SOURCE
#include "config.h"
#include <bm.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    FILE *fin = fopen(PROJECT_SOURCE_DIR "/ネザー要塞シード.txt", "r");
    if (fin == NULL)
    {
        perror("");
        return 1;
    }
    char buf[BUFSIZ] = "";
    char *catch = NULL;
    long a = 0;
    while (fgets(buf, BUFSIZ, fin) != NULL)
    {
        a = strtol(buf, &catch, 10);
        if (a >= 0)
        {
            break;
        }
        printf("%ld\n", a & 0xffffffffffffL);
    }
    fclose(fin);

    return 0;
}
