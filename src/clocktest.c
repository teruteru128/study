
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define a(a) {#a, a}

struct cid
{
    const char name[32];
    clockid_t id;
};

void clocktest(void)
{
    struct timespec res;
    struct timespec spec;
    static const struct cid cid[] = {a(CLOCK_REALTIME),
                                     a(CLOCK_REALTIME_COARSE),
                                     a(CLOCK_MONOTONIC),
                                     a(CLOCK_MONOTONIC_COARSE),
                                     a(CLOCK_MONOTONIC_RAW),
                                     a(CLOCK_BOOTTIME),
                                     a(CLOCK_PROCESS_CPUTIME_ID),
                                     a(CLOCK_THREAD_CPUTIME_ID),
                                     a(CLOCK_REALTIME_ALARM),
                                     a(CLOCK_BOOTTIME_ALARM),
                                     {"No.10, what is this?", 10},
                                     a(CLOCK_TAI),
                                     {"", 0}};
    int tmperrno = 0;
    char *errmsg = NULL;
    int haveerr = 0;
    locale_t japanese_locale = newlocale(LC_CTYPE_MASK | LC_MESSAGES_MASK, "ja_JP.UTF-8", NULL);
    if (japanese_locale == NULL)
    {
        perror("newlocale");
        exit(EXIT_FAILURE);
    }
    if(japanese_locale == LC_GLOBAL_LOCALE)
    {
        perror("japanse_locale is global_locale");
        return;
    }
    for (size_t i = 0; cid[i].name[0] != '\0'; i++)
    {
        haveerr = 0;
        res.tv_sec = -1;
        res.tv_nsec = 0;
        if (clock_getres(cid[i].id, &res) != 0)
        {
            tmperrno = errno;
            errmsg = strerror_l(tmperrno, japanese_locale);
            fprintf(stdout, "clock_getres(%d) : %s\n", cid[i].id, errmsg);
            haveerr = 1;
            continue;
        }
        spec.tv_sec = -1;
        spec.tv_nsec = 0;
        if (clock_gettime(cid[i].id, &spec))
        {
            tmperrno = errno;
            errmsg = strerror_l(tmperrno, japanese_locale);
            if (haveerr != 0)
                fputs(", ", stdout);
            fprintf(stdout, "clock_gettime(%d) : %s\n", cid[i].id, errmsg);
            haveerr = 1;
        }
        if (haveerr != 0)
            fputs(", ", stdout);
        printf("%s: %ld.%09ld, %ld.%09ld\n", cid[i].name, res.tv_sec,
               res.tv_nsec, spec.tv_sec, spec.tv_nsec);
    }
    fputc('\n', stdout);
    struct timespec cur;
    clock_gettime(CLOCK_MONOTONIC, &cur);
    printf("%ld.%09ld, %lf\n", cur.tv_sec, cur.tv_nsec, cur.tv_sec / 86400.);
    clock_gettime(CLOCK_MONOTONIC, &cur);
    printf("%ld.%09ld\n", cur.tv_sec, cur.tv_nsec);
    freelocale(japanese_locale);
}
