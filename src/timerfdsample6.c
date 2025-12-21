#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

#define MAX_EVENTS 2

// timerfdを作成し、インターバルを設定する関数
int create_timer(int seconds)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1)
    {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec new_value;
    new_value.it_value.tv_sec = seconds; // 初回発火までの時間
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = seconds; // 以降のインターバル
    new_value.it_interval.tv_nsec = 0;

    if (timerfd_settime(tfd, 0, &new_value, NULL) == -1)
    {
        perror("timerfd_settime");
        exit(EXIT_FAILURE);
    }
    return tfd;
}

int timerfdsample6(void)
{
    int epfd = epoll_create1(0);
    if (epfd == -1)
    {
        perror("epoll_create1");
        return 1;
    }

    // 5秒と3秒のタイマーを作成
    int tfd5 = create_timer(5);
    int tfd3 = create_timer(3);

    // epollに登録
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = tfd5;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd5, &ev);

    ev.data.fd = tfd3;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd3, &ev);

    printf("Monitoring timers (5s and 3s)... Ctrl+C to stop\n");

    while (1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            break;
        }

        for (int n = 0; n < nfds; n++)
        {
            uint64_t exp;
            // timerfdはreadしないと次のイベントが発生し続けない
            ssize_t s = read(events[n].data.fd, &exp, sizeof(uint64_t));
            if (s != sizeof(uint64_t))
                continue;

            if (events[n].data.fd == tfd5)
            {
                printf("[Timer A] 5 seconds elapsed! (total exp: %" PRIu64 ")\n", exp);
            }
            else if (events[n].data.fd == tfd3)
            {
                printf("[Timer B] 3 seconds elapsed! (total exp: %" PRIu64 ")\n", exp);
            }
        }
    }

    close(tfd5);
    close(tfd3);
    close(epfd);
    return 0;
}
