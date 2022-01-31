
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "server.h"
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_BUF_SIZE 1024
#define MAX_EVENTS 16

/**
 * @brief listening socket
 * リッスンソケットはどこで保持すべきなんだろうか？
 */
static int listensocket = -1;

// finish flag
volatile int running = 1;

int startServer()
{
    return 0;
}

/*
 * listen host name
 * listen family
 * listen port
 *
 * killsignal : sigint
 * reloadsignal : sighup
 */
static int get_socket(const char *port)
{
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    struct addrinfo hints = { 0 };
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int ecode;
    struct addrinfo *res;
    if ((ecode = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "failed getaddrinfo() %s\n", gai_strerror(ecode));
        return -1;
    }

    int sock;
    for (struct addrinfo *ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock < 0)
        {
            continue;
        }

        if (bind(sock, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            close(sock);
            sock = -1;
            continue;
        }

        if (listen(sock, SOMAXCONN) < 0)
        {
            close(sock);
            sock = -1;
            continue;
        }
    }
    freeaddrinfo(res);

    return sock;
}

int init_server(char *argv)
{
    if (listensocket != -1)
    {
        return 1;
    }
    int sock = get_socket(argv);
    if (sock < 0)
        return sock;
    listensocket = sock;
    return 0;
}

static void echo_back(int sock)
{
    char buf1[MAX_BUF_SIZE];
    char buf2[MAX_BUF_SIZE];
    uint32_t *ptr = NULL, tmp;
    ssize_t len;
    int flg = 0;
    struct pollfd fds = { 0 };
    fds.fd = sock;
    fds.events = POLLIN;
    fds.revents = 0;
    struct timespec spec = { 3, 0 };
    sigset_t sigmask;
    sigemptyset(&sigmask);
    int selret = 0;

    while (running)
    {
        memset(buf1, 0, MAX_BUF_SIZE);
        len = recv(sock, buf1, sizeof(buf1), 0);
        if (len == -1)
        {
            perror("recv() failed.");
            close(sock);
            break;
        }
        else if (len == 0)
        {
            fprintf(stderr, "connection closed by remote host.\n");
            break;
        }
        ptr = (uint32_t *)buf1;
        tmp = ntohl(*ptr);
        if (tmp == (uint32_t)-1)
        {
            fprintf(stderr, "exit command\n");
            flg = -1;
        }
        else
        {
            tmp = (tmp * 4) % 3779;
        }
        memset(buf2, 0, MAX_BUF_SIZE);
        snprintf(buf2, MAX_BUF_SIZE, "%u", tmp);
        *ptr = htonl(tmp);

        if (send(sock, buf2, (size_t)len, 0) != len)
        {
            perror("send() failed.");
            break;
        }
        if (flg == -1)
        {
            break;
        }
    }
}

void *do_service(void *arg)
{
    struct service_arg *arg2 = (struct service_arg *)arg;
    if (init_server(arg2->port) < 0)
    {
        fprintf(stderr, "init_server failure.\n");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev = { 0 };
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = listensocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listensocket, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    struct timespec spec = { 3, 0 };

    sigset_t sigmask;
    sigemptyset(&sigmask);

    struct epoll_event events[MAX_EVENTS] = { 0 };
    struct sockaddr_storage from_sock_addr = { 0 };
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    int selret = 0;
    int nfds, n;
    int conn_sock = -1;
    char name[NI_MAXHOST], service[NI_MAXSERV];
    while (running)
    {
        nfds = epoll_pwait(epollfd, events, MAX_EVENTS, -1, &sigmask);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (n = 0; n < nfds; n++)
        {
            if (events[n].data.fd == listensocket)
            {
                conn_sock
                    = accept(listensocket, (struct sockaddr *)&from_sock_addr,
                             &addr_len);
                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                getnameinfo((struct sockaddr *)&from_sock_addr, addr_len, name,
                            NI_MAXHOST, service, NI_MAXSERV,
                            NI_NUMERICHOST | NI_NUMERICSERV);

                fprintf(stderr, "port is %s\n", service);
                fprintf(stderr, "host is %s\n", name);
                // setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                // do_use_fd(events[n].data.fd);
            }
        }
    }
    close(listensocket);
    listensocket = -1;
}

/**
 * TODO: グローバル変数をフラグにして終了
 */
void close_server() { running = 0; }
