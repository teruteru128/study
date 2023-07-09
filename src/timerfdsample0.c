
#define _GNU_SOURCE 1
#include "timerfdsample0.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

/**
 * @brief eventfd sample.
 *
 * eventfd
 * timerfd
 * signalfd
 * epoll
 *
 * http://www.eyes-software.co.jp/news/archives/9
 * https://web.archive.org/web/20220214123933/http://www.eyes-software.co.jp/news/archives/9
 * Linuxにおけるウェイト処理
 * - sleep
 * - usleep
 * - nanosleep
 * - select(2) のタイムアウト指定
 * - settimer
 * - timerfd
 *   timerfdはsleepとかusleep, nanosleepに比べると手間がかかりますねー
 *   そりゃfdだからねえ
 * - pthread_cond_timedwait
 * - timer_create
 * clock_getcpuclockid
 * pthread_getcpuclockid
 * sigevent
 * signal
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
 * 5分のtimerfdと3分のtimerfdの2つをepollで監視する
 *
 * @return int
 */
int timerfdsample0(void)
{
    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    struct itimerspec spec;
    // 現在時刻を取得
    clock_gettime(CLOCK_REALTIME, &spec.it_value);
    // it_valueを両方0にするとタイマー停止
    // タイマー満了時間の初期値用に繰り上げ
    spec.it_value.tv_sec++;
    spec.it_value.tv_nsec = 0;
    // it_intervalを両方0にすると繰り返しタイマー停止
    spec.it_interval.tv_sec = 1;
    spec.it_interval.tv_nsec = 0;
    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &spec, NULL) != 0)
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
    int ret = EXIT_SUCCESS;
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
