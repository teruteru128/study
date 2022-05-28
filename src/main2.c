
#define _GNU_SOURCE
#include "config.h"

#include <byteswap.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

int main(int argc, char const *argv[])
{
    char *lo = setlocale(LC_ALL, "");

    int epollfd = epoll_create(1);
    struct epoll_event ev = { 0 };
    ev.events = EPOLLIN;
    ev.data.fd = 0;
    // epoll は edge-poll の略？
    // epoll_fd の fd に指定するものと ev.data.fd に指定するものでなんで2つあるんや
    //fcntl(0, F_SETFL, O_NONBLOCK);
    //epoll_ctl(epollfd, EPOLL_CTL_ADD, 0, &ev);
    //epoll_ctl(epollfd, EPOLL_CTL_DEL, 0, &ev);
    close(epollfd);
    printf("%zu\n", sizeof(struct epoll_event));

    return 0;
}
