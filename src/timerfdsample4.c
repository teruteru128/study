
#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "timerfdsample4.h"
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <time.h>

static volatile sig_atomic_t running = 1;

static void handler(int sig, siginfo_t *info, void *ctx)
{
    running = 0;
    (void)sig;
    (void)info;
    (void)ctx;
}

int timerfdsample4(void)
{
    char *lo = setlocale(LC_ALL, "");
    struct sigaction action = { 0 };
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;
    if (sigaction(SIGINT, &action, NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        return EXIT_FAILURE;
    }

    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        err(EXIT_FAILURE, "timerfd_create");
    }
    printf("タイマーを作成しました。\n");

    struct itimerspec spec;
    clock_gettime(CLOCK_REALTIME, &spec.it_value);
    if (spec.it_value.tv_nsec != 0)
    {
        spec.it_value.tv_nsec = 0;
        spec.it_value.tv_sec++;
    }
    spec.it_value.tv_sec = ((spec.it_value.tv_sec + 599) / 600) * 600;
    spec.it_interval.tv_sec = 600;
    spec.it_interval.tv_nsec = 0;

    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &spec, NULL) != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        exit(EXIT_FAILURE);
    }
    char buf[BUFSIZ] = "";
    struct tm times;
    localtime_r(&spec.it_value.tv_sec, &times);
    strftime(buf, BUFSIZ, "%Ex %EX %z", &times);
    printf("タイマーに起動時刻ををセットしました。\n");
     printf("初回起動時間 : %s\n", buf);
    uint64_t exp;
    ssize_t size;
    struct timespec current;
    while (running)
    {
        size = read(timerfd, &exp, sizeof(uint64_t));
        if (size < 8)
        {
            perror("read error");
        }
        clock_gettime(CLOCK_REALTIME, &current);
        printf("%zd, %ld\n", size, current.tv_sec);
    }

    close(timerfd);
    timerfd = -1;
    printf("タイマーをクリアしました。\n");

    return 0;
}
