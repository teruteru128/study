
#define _GNU_SOURCE 1
#include "timerfdsample0.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

int timerfdsample1(void)
{
    fputs("10分おきに送信します\n", stdout);
    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    struct itimerspec spec;
    clock_gettime(CLOCK_REALTIME, &spec.it_value);
    spec.it_value.tv_sec = ((spec.it_value.tv_sec + 599) / 600) * 600;
    spec.it_value.tv_nsec = 0;
    // it_intervalを両方0にすると繰り返しタイマーオフ
    spec.it_interval.tv_sec = 5;
    spec.it_interval.tv_nsec = 0;
    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &spec, NULL) != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        return EXIT_FAILURE;
    }

    // expiration
    uint64_t exp;
    ssize_t r = 0;
    int ret = EXIT_SUCCESS;
    struct timespec current;
    while (1)
    {
        // recvで読み込むと失敗を返す
        // 一度にuint64_tを2つ分以上readさせても1回分の時間しか停止しないし1つしか書き込まれない
        r = read(timerfd, &exp, sizeof(uint64_t));
        if (r != sizeof(uint64_t))
        {
            perror("recv");
            ret = EXIT_FAILURE;
            break;
        }
        clock_gettime(CLOCK_REALTIME_COARSE, &current);
        printf("%ld.%09ld\n", current.tv_sec, current.tv_nsec);
    }

    close(timerfd);
    return ret;
}
