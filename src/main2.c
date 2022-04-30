
#define _GNU_SOURCE
#include "config.h"

#include <bm.h>
#include <byteswap.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

int main(int argc, char const *argv[])
{
    char *lo = setlocale(LC_ALL, "");

    char buf[256] = "";
    char buf2[BUFSIZ] = "";
    struct timespec spec;
    struct tm tm;
    for (size_t i = 0; i < 8192; i += 512)
    {
        clock_gettime(CLOCK_REALTIME, &spec);
        localtime_r(&spec.tv_sec, &tm);
        strftime(buf, 256, "%Ex %EX", &tm);
        snprintf(buf2 + i, 512, "%s.%09ld", buf, spec.tv_nsec);
    }
    for (size_t i = 0; i < 8192; i += 512)
    {
        printf("%s\n", buf2 + i);
    }

    return 0;
}
