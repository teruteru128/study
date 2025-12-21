
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>

struct data
{
    int fd;
    int seconds;
};

int create_timerfd(struct timespec *spec, int seconds)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1)
    {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }
    struct itimerspec new_value;
    new_value.it_value.tv_sec = seconds;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = seconds;
    new_value.it_interval.tv_nsec = 0;
    if (timerfd_settime(tfd, 0, &new_value, NULL) == -1)
    {
        perror("timerfd_settime");
        exit(EXIT_FAILURE);
    }
    return tfd;
}

#define LIST_ENT 16
#define MAX_EVENTS 16

/*
epollと複数timerfdによる連携のサンプル
3分タイマーと5分タイマーをepollで監視する
複数のタイマーを同時に監視できるってすげえよなぁ
epoll fdさえ保持しておけばlisten socketとかaccepted
socketとか別に保持しておかなくてもいいのか
*/
int timerfdsample5()
{
    int epfd = epoll_create1(0);
    struct epoll_event ev;
    struct data data[LIST_ENT];
    struct timespec s;
    clock_gettime(CLOCK_REALTIME, &s);
    struct timespec d;
    /*
     */
    int list[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59};
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    ev.events = EPOLLIN;
    for (int i = 0; i < LIST_ENT; i++)
    {
        data[i].fd = create_timerfd(&spec, list[i]);
        data[i].seconds = list[i];
        ev.data.ptr = &(data[i]);
        // ev.data.fd = data[i].fd;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, data[i].fd, &ev);
        if (ret != 0)
        {
            perror("epoll_ctl_add failed");
            return 1;
        }
    }
    struct epoll_event events[MAX_EVENTS];
    int nfds = 0;
    uint64_t expired = 0;
    char buf[512];
    struct tm tm;
    printf("Monitoring timers ... Ctrl+C to stop\n");
    while (1)
    {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds < 0)
        {
            perror("epoll_wait");
            break;
        }

        for (size_t i = 0; i < nfds; i++)
        {
            ssize_t s = read(((struct data *)events[i].data.ptr)->fd, &expired, sizeof(uint64_t));
            if (s != sizeof(uint64_t))
                continue;

            clock_gettime(CLOCK_REALTIME, &d);
            localtime_r(&d.tv_sec, &tm);
            strftime(buf, 512, "%Y/%m/%d %T", &tm);
            printf("[%s.%09" PRId64 "]%d秒!, %" PRIu64 "\n", buf, (int64_t)d.tv_nsec, ((struct data *)events[i].data.ptr)->seconds, expired);
        }
    }

    return 0;
}
