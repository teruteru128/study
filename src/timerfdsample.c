
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <inttypes.h>

/**
 * eventfd sample.
 * 
 * eventfd
 * timerfd
 * signalfd
 * 
 * Linuxにおけるウェイト処理
 * - sleep
 * - usleep
 * - nanosleep
 * - selectのタイムアウト指定
 * - settimer
 * - timerfd
 * - pthread_cond_timedwait
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    struct timespec cur;
    clock_gettime(CLOCK_MONOTONIC, &cur);
    printf("%ld.%09ld\n", cur.tv_sec, cur.tv_nsec);
    printf("%lf\n", (double)cur.tv_sec / 86400);
    clock_gettime(CLOCK_REALTIME, &cur);
    struct itimerspec spec;
    spec.it_value.tv_sec = cur.tv_sec + 3;
    spec.it_value.tv_nsec = cur.tv_nsec;
    // it_intervalを両方0にすると繰り返しタイマーオフ
    spec.it_interval.tv_sec = 1;
    spec.it_interval.tv_nsec = 0;

    int timer = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timer < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    int ret = timerfd_settime(timer, TFD_TIMER_ABSTIME, &spec, NULL);
    if (ret != 0)
    {
        perror("timerfd_settime");
        close(timer);
        return EXIT_FAILURE;
    }

    // expiration
    uint64_t exp = 0;
    // max expiration
    uint64_t max_exp = 16;
    // total expiration
    uint64_t tot_exp;
    ssize_t r = 0;
    ret = 0;
    for (tot_exp = 0; tot_exp < max_exp;)
    {
        // recvで読み込むと失敗を返す
        r = read(timer, &exp, sizeof(uint64_t));
        if (r != sizeof(uint64_t))
        {
            perror("recv");
            ret = EXIT_FAILURE;
            break;
        }
        tot_exp += exp;
        printf("read : %" PRId64 " , total : %" PRIu64 "\n", exp, tot_exp);
    }

    close(timer);
    return ret;
}
