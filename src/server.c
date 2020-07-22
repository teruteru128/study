
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include "server.h"
#define MAX_BUF_SIZE 1024

static int sock = -1;
static int running = 1;

/*
 * listen host name
 * listen family
 * listen port
 */
int get_socket(const char *port)
{
    struct addrinfo hints, *res;
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
    if ((ecode = getnameinfo(res->ai_addr, res->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
    {
        fprintf(stderr, "failed getnameinfo() %s\n", gai_strerror(ecode));
        freeaddrinfo(res);
        return -1;
    }
    fprintf(stdout, "port is %s\n", sbuf);
    fprintf(stdout, "host is %s\n", hbuf);

    if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("socket() failed.");
        freeaddrinfo(res);
        return -1;
    }

    if (bind(sock, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("bind() failed.");
        close(sock);
        freeaddrinfo(res);
        return -1;
    }

    if (listen(sock, SOMAXCONN) < 0)
    {
        perror("listen() failed.");
        close(sock);
        freeaddrinfo(res);
        return -1;
    }

    return sock;
}

int init_server(char *argv)
{
    if (sock == -1)
    {
        int s = get_socket(argv);
        if (s < 0)
            return s;
        sock = s;
    }
    return sock;
}

void echo_back(int sock)
{

    char buf1[MAX_BUF_SIZE];
    char buf2[MAX_BUF_SIZE];
    uint32_t *ptr = NULL, tmp;
    ssize_t len;
    int flg = 0;
    struct pollfd fds = {sock, POLLIN | POLLERR, 0};
    struct timespec spec = {3, 0};
    sigset_t sigmask;
    sigemptyset(&sigmask);
    int selret = 0;

    for (; running;)
    {
        selret = ppoll(&fds, 1, &spec, &sigmask);
        if (selret == -1)
        {
            perror("select");
            close(sock);
            return;
        }
        if (selret == 0)
        {
            continue;
        }
        if (fds.revents & POLLERR)
        {
            perror("isset failed");
            close(sock);
            return;
        }
        memset(buf1, 0, MAX_BUF_SIZE);
        if ((len = recv(sock, buf1, sizeof(buf1), 0)) == -1)
        {
            perror("recv() failed.");
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

static inline void do_concrete_service(int sock)
{
    echo_back(sock);
}

void *do_service(void *arg)
{
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_storage from_sock_addr;
    int acc_sock = -1;
    socklen_t addr_len = sizeof(from_sock_addr);
    struct pollfd fds = {sock, POLLIN | POLLERR, 0};
    struct timespec spec = {3, 0};
    sigset_t sigmask;
    sigfillset(&sigmask);
    int selret = 0;
    for (;;)
    {
        selret = ppoll(&fds, 1, &spec, &sigmask);
        if (selret == -1)
        {
            perror("select");
            close(sock);
            return NULL;
        }
        if (selret == 0)
        {
            continue;
        }
        if (fds.revents & POLLERR)
        {
            perror("isset failed");
            close(sock);
            return NULL;
        }
        if ((acc_sock = accept(sock, (struct sockaddr *)&from_sock_addr, &addr_len)) != -1)
        {
            getnameinfo((struct sockaddr *)&from_sock_addr, addr_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);

            fprintf(stderr, "port is %s\n", sbuf);
            fprintf(stderr, "host is %s\n", hbuf);

            do_concrete_service(acc_sock);

            close(acc_sock);
        }
        else
        {
            perror("accept() failed.");
            continue;
        }
    }
    close(sock);
    sock = -1;
}

/*
 * TODO: グローバル変数をフラグにして終了
 */
void close_server()
{
    running = 0;
}
