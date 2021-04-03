
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <inttypes.h>

#define TIME_FORMAT_BUFFER_SIZE 64

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

    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    struct itimerspec spec;
    spec.it_value.tv_sec = time(NULL) + 1;
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
    FILE *timerf = fdopen(timerfd, "rbe");
    if (timerf == NULL)
    {
        perror("fdopen");
        close(timerfd);
        return EXIT_FAILURE;
    }

    // expiration
    uint64_t exp[2] = {0, 0};
    // max expiration
    const uint64_t max_exp = 16;
    // total expiration
    uint64_t tot_exp = 0;
    size_t r = 0;
    struct tm tm;
    char format[TIME_FORMAT_BUFFER_SIZE];
    char formattedTime[TIME_FORMAT_BUFFER_SIZE];
    while (tot_exp < max_exp)
    {
        // recvで読み込むと失敗を返す
        // FILEでラップしてfreadはセーフ
        // 一度にuint64_tを2つ分以上freadさせると指定した個数分だけウェイトがかかり、すべての領域に満了回数が書き込まれる
        // freadがこの挙動をすることに依存すべきでないと思う
        r = fread(exp, sizeof(uint64_t), 1, timerf);
        clock_gettime(CLOCK_REALTIME, &cur);
        if (r != 1)
        {
            perror("recv");
            ret = EXIT_FAILURE;
            break;
        }
        localtime_r(&cur.tv_sec, &tm);
        strftime(format, TIME_FORMAT_BUFFER_SIZE, "%FT%T.%%09ld%z", &tm);
        snprintf(formattedTime, TIME_FORMAT_BUFFER_SIZE, format, cur.tv_nsec);
        tot_exp += exp[0] + exp[1];
        printf("%s, read : %" PRId64 ", %" PRId64 " , total : %" PRIu64 "\n", formattedTime, exp[0], exp[1], tot_exp);
    }

    fclose(timerf);
    return ret;
}
