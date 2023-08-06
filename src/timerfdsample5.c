
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

struct data
{
    int fd;
    int minutes;
};

int create_timerfd(struct timespec *spec, int minutes)
{
    int fd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec ispec;
    long interval = minutes * 60;
    ispec.it_value.tv_sec
        = ((spec->tv_sec + interval - 1) / interval) * interval;
    ispec.it_value.tv_nsec = 0;
    ispec.it_interval.tv_sec = interval;
    ispec.it_interval.tv_nsec = 0;
    timerfd_settime(fd, TFD_TIMER_ABSTIME, &ispec, NULL);
    return fd;
}

#define LIST_ENT 6

/*
epollと複数timerfdによる連携のサンプル
3分タイマーと5分タイマーをepollで監視する
複数のタイマーを同時に監視できるってすげえよなぁ
epoll fdさえ保持しておけばlisten socketとかaccepted
socketとか別に保持しておかなくてもいいのか
*/
int timerfdsample5()
{
    int efd = epoll_create1(0);
    struct epoll_event event = { 0 };
    struct data *data = malloc(sizeof(struct data) * LIST_ENT);
    struct timespec s;
    clock_gettime(CLOCK_REALTIME, &s);
    struct timespec d;
    int list[] = { 3, 5, 7, 11, 13, 17 };
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    for (int i = 0; i < LIST_ENT; i++)
    {
        (data + i)->fd = create_timerfd(&spec, list[i]);
        (data + i)->minutes = list[i];
        event.events = EPOLLIN;
        event.data.ptr = data + i;
        int ret = epoll_ctl(efd, EPOLL_CTL_ADD, (data + i)->fd, &event);
        if (ret != 0)
        {
            perror("epoll_ctl 1");
            return 1;
        }
    }
    struct epoll_event events[16];
    int c = 0;
    uint64_t expired = 0;
    char buf[512];
    struct tm tm;
    while (1)
    {
        c = epoll_wait(efd, events, 16, 50);
        if (c < 0)
        {
            perror("epoll_wait");
        }
        for (size_t i = 0; i < c; i++)
        {
            clock_gettime(CLOCK_REALTIME, &d);
            localtime_r(&d.tv_sec, &tm);
            strftime(buf, 512, "%Y/%m/%d %T", &tm);
            read(((struct data *)events[i].data.ptr)->fd, &expired, 8);
            printf("[%s]%d分!, %lu\n", buf,
                   ((struct data *)events[i].data.ptr)->minutes, expired);
        }
    }

    return 0;
}
