
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define a(a)  \
    {         \
#a, a \
    }

struct cid
{
    const char name[32];
    clockid_t id;
};

void clocktest()
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
                                     // {"No.10, what is this?", 10},
                                     a(CLOCK_TAI),
                                     {"", 0}};
    for (size_t i = 0; cid[i].name[0] != '\0'; i++)
    {
        if (clock_getres(cid[i].id, &res) != 0)
        {
            perror("clock_getres");
        }
        if (clock_gettime(cid[i].id, &spec))
        {
            perror("clock_gettime");
        }
        printf("%s : %ld.%09ld, %ld.%09ld\n", cid[i].name, res.tv_sec, res.tv_nsec, spec.tv_sec, spec.tv_nsec);
    }
    fputc('\n', stdout);
    struct timespec cur;
    clock_gettime(CLOCK_MONOTONIC, &cur);
    printf("%ld.%09ld, %lf\n", cur.tv_sec, cur.tv_nsec, cur.tv_sec / 86400.);
    clock_gettime(CLOCK_MONOTONIC, &cur);
    printf("%ld.%09ld\n", cur.tv_sec, cur.tv_nsec);
}
