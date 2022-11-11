
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <time.h>

/**
 * @brief result = time1 - time0
 *
 * @param result
 * @param time1
 * @param time0
 * @return struct timespec*
 */
struct timespec *difftimespec(struct timespec *result, struct timespec *time1,
                              struct timespec *time0)
{
    if ((time1->tv_nsec - time0->tv_nsec) < 0)
    {
        result->tv_sec = time1->tv_sec - time0->tv_sec - 1;
        result->tv_nsec = time1->tv_nsec - time0->tv_nsec + 1000000000;
    }
    else
    {
        result->tv_sec = time1->tv_sec - time0->tv_sec;
        result->tv_nsec = time1->tv_nsec - time0->tv_nsec;
    }
    // FIXME: どっちのほうがmore betterなんだろうね？
    /*
    result->tv_sec = time1->tv_sec - time0->tv_sec;
    result->tv_nsec = time1->tv_nsec - time0->tv_nsec;
    if (result->tv_nsec < 0)
    {
        result->tv_sec--;
        result->tv_nsec += 1000000000;
    }
    */
    return result;
}
