
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
 * @brief eventfd sample.
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
 *   timerfdはsleepとかusleep, nanosleepに比べると手間がかかりますねー
 *   そりゃfdだからねえ
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
    printf("%ld.%09ld, %lf\n", cur.tv_sec, cur.tv_nsec, (double)cur.tv_sec / 86400);
    clock_gettime(CLOCK_MONOTONIC, &cur);
    printf("%ld.%09ld\n", cur.tv_sec, cur.tv_nsec);

    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    struct itimerspec spec;
    spec.it_value.tv_sec = 1;
    spec.it_value.tv_nsec = 0;
    // it_intervalを両方0にすると繰り返しタイマーオフ
    spec.it_interval.tv_sec = 0;
    spec.it_interval.tv_nsec = 100000;
    int ret = timerfd_settime(timerfd, 0, &spec, NULL);
    if (ret != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        return EXIT_FAILURE;
    }

    // expiration
    uint64_t exp = 0;
    // max expiration
    const uint64_t max_exp = 16000;
    // total expiration
    uint64_t tot_exp;
    ssize_t r = 0;
    for (tot_exp = 0; tot_exp < max_exp;)
    {
        // recvで読み込むと失敗を返す
        r = read(timerfd, &exp, sizeof(uint64_t));
        if (r != sizeof(uint64_t))
        {
            perror("recv");
            ret = EXIT_FAILURE;
            break;
        }
        tot_exp += exp;
        printf("read : %" PRId64 " , total : %" PRIu64 "\n", exp, tot_exp);
    }

    close(timerfd);
    return ret;
}
