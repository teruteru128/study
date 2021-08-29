
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
 * epoll
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
 * データ型とか
 * time_t
 * <time.h>
 * time(2)
 * 
 * struct timespec
 * <time.h>
 * timespec_get()
 * 
 * struct tm
 * 
 * char*
 * 
 * timerfdを使って一定時間ごとにpthread_cond_signalとか
 * pthread_cond_broadcastとか使ってシグナルを送信するとか……
 * うーん邪悪
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    struct timespec cur;

    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    clock_gettime(CLOCK_REALTIME, &cur);
    struct itimerspec spec;
    spec.it_value.tv_sec = cur.tv_sec + 1;
    spec.it_value.tv_nsec = 0;
    // it_intervalを両方0にすると繰り返しタイマーオフ
    spec.it_interval.tv_sec = 1;
    spec.it_interval.tv_nsec = 0;
    int ret = timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &spec, NULL);
    if (ret != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        return EXIT_FAILURE;
    }

    // expiration
    uint64_t exp;
    // max expiration
    const uint64_t max_exp = 16;
    // total expiration
    uint64_t tot_exp = 0;
    ssize_t r = 0;
    while (tot_exp < max_exp)
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
        tot_exp += exp;
        printf("read : %" PRId64 ", total : %" PRIu64 "\n", exp, tot_exp);
    }

    close(timerfd);
    return ret;
}
