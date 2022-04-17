
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
    perror("");
    printf("%s\n", lo);

    struct tm deadline_tm = { 0 };
    deadline_tm.tm_sec = 0;
    deadline_tm.tm_min = 0;
    deadline_tm.tm_hour = 0;
    deadline_tm.tm_mday = 27;
    deadline_tm.tm_mon = 4;
    deadline_tm.tm_year = 2022 - 1900;
    deadline_tm.tm_wday = 0;
    deadline_tm.tm_yday = 0;
    deadline_tm.tm_isdst = 0;
    deadline_tm.tm_gmtoff = 0;
    deadline_tm.tm_zone = NULL;

    time_t deadline = mktime(&deadline_tm);

    time_t now = time(NULL);

    printf("%lf\n", difftime(deadline, now));
    printf("%zu\n", sizeof(struct tm));

    return 0;
}
