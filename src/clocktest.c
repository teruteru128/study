
#include <time.h>

#define a(a)  \
    {         \
#a, a \
    }

struct cid
{
    const char *name;
    clockid_t id;
};

void clocktest()
{
    struct timespec res;
    struct timespec spec;
    struct cid cid[] = {a(CLOCK_REALTIME),
                        a(CLOCK_REALTIME_COARSE),
                        a(CLOCK_MONOTONIC),
                        a(CLOCK_MONOTONIC_COARSE),
                        a(CLOCK_MONOTONIC_RAW),
                        a(CLOCK_BOOTTIME),
                        a(CLOCK_PROCESS_CPUTIME_ID),
                        a(CLOCK_THREAD_CPUTIME_ID),
                        {NULL, 0}};
    for (size_t i = 0; cid[i].name != NULL; i++)
    {
        clock_getres(cid[i].id, &res);
        clock_gettime(cid[i].id, &spec);
        printf("%s : %ld.%09ld, %ld.%09ld\n", cid[i].name, res.tv_sec, res.tv_nsec, spec.tv_sec, spec.tv_nsec);
    }
}