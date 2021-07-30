
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
static int running = 1;

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
    struct addrinfo hints, *res, *ptr;
    int ecode, sock;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if ((ecode = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "failed getaddrinfo() %s\n", gai_strerror(ecode));
        return -1;
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
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

int init_server(int *listensocket, char *argv)
{
    if (listensocket == NULL)
    {
        return 1;
    }
    int sock = get_socket(argv);
    if (sock < 0)
        return sock;
    *listensocket = sock;
    return 0;
}

static void echo_back(int sock)
{
    char buf1[MAX_BUF_SIZE];
    char buf2[MAX_BUF_SIZE];
    uint32_t *ptr = NULL, tmp;
    ssize_t len;
    int flg = 0;
    // TODO: epollに書き換え
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

static void do_concrete_service(int sock) { echo_back(sock); }

void *do_service(void *arg)
{
    struct service_arg *arg2 = (struct service_arg *)arg;
    int listen_sock = arg2->listen_socket;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_storage from_sock_addr;
    int conn_sock = -1;
    socklen_t addr_len = sizeof(from_sock_addr);
    struct epoll_event ev = { 0 }, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    struct timespec spec = { 3, 0 };

    sigset_t sigmask;
    sigfillset(&sigmask);

    int selret = 0;
    int nfds, n;
    while (running)
    {
        nfds = epoll_pwait(epollfd, events, MAX_EVENTS, -1, NULL);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (n = 0; n < nfds; n++)
        {
            if (events[n].data.fd == listen_sock)
            {
                conn_sock
                    = accept(listen_sock, (struct sockaddr *)&from_sock_addr,
                             &addr_len);
                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                getnameinfo((struct sockaddr *)&from_sock_addr, addr_len, hbuf,
                            sizeof(hbuf), sbuf, sizeof(sbuf),
                            NI_NUMERICHOST | NI_NUMERICSERV);

                fprintf(stderr, "port is %s\n", sbuf);
                fprintf(stderr, "host is %s\n", hbuf);
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
    struct pollfd fds = { 0 };
    fds.fd = listen_sock;
    fds.events = POLLIN;
    fds.revents = 0;
    for (;;)
    {
        selret = ppoll(&fds, 1, &spec, &sigmask);
        if (selret == -1)
        {
            perror("select");
            close(listen_sock);
            return NULL;
        }
        if (selret == 0)
        {
            continue;
        }
        if (fds.revents & POLLERR)
        {
            perror("isset failed");
            close(listen_sock);
            return NULL;
        }
        if ((conn_sock = accept(listen_sock,
                                (struct sockaddr *)&from_sock_addr, &addr_len))
            != -1)
        {
            getnameinfo((struct sockaddr *)&from_sock_addr, addr_len, hbuf,
                        sizeof(hbuf), sbuf, sizeof(sbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);

            fprintf(stderr, "port is %s\n", sbuf);
            fprintf(stderr, "host is %s\n", hbuf);

            do_concrete_service(conn_sock);

            close(conn_sock);
        }
        else
        {
            perror("accept() failed.");
            continue;
        }
    }
    close(listen_sock);
    listen_sock = -1;
}

/**
 * TODO: グローバル変数をフラグにして終了
 */
void close_server() { running = 0; }
