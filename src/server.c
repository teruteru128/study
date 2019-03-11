
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "server.h"

static int sock = 0;

int get_socket(const char *port)
{
    struct addrinfo hints, *res;
    int ecode, sock;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_UNSPEC;
    if ((ecode = getaddrinfo("localhost", port, &hints, &res)) != 0)
    {
        fprintf(stderr, "failed getaddrinfo() %s\n", gai_strerror(ecode));
        goto failure_1;
    }
    if ((ecode = getnameinfo(res->ai_addr, res->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
    {
        fprintf(stderr, "failed getnameinfo() %s\n", gai_strerror(ecode));
        goto failure_2;
    }
    fprintf(stdout, "port is %s\n", sbuf);
    fprintf(stdout, "host is %s\n", hbuf);

    if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("socket() failed.");
        goto failure_2;
    }

    if (bind(sock, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("bind() failed.");
        goto failure_3;
    }

    if (listen(sock, SOMAXCONN) < 0)
    {
        perror("listen() failed.");
        goto failure_3;
    }

    return sock;
failure_3:
    close(sock);
failure_2:
    freeaddrinfo(res);
failure_1:
    return -1;
}
int init_server(char *argv)
{
    if (sock==0)
    {
        sock = get_socket(argv);
    }
    return sock == -1 ? -1 : sock;
}
void echo_back(int sock)
{

    char buf[MAX_BUF_SIZE];
    uint32_t *ptr = NULL, tmp;
    ssize_t len;
    int flg = 0;

    for (;;)
    {
        if ((len = recv(sock, buf, sizeof(buf), 0)) == -1)
        {
            perror("recv() failed.");
            break;
        }
        else if (len == 0)
        {
            fprintf(stderr, "connection closed by remote host.\n");
            break;
        }
        ptr = (uint32_t *)buf;
        tmp = ntohl(*ptr);
        if (tmp == -1)
        {
            fprintf(stderr, "exit command\n");
            flg = -1;
        }
        else
        {
            tmp = (tmp * 4) % 3779;
        }
        *ptr = htonl(tmp);

        if (send(sock, buf, (size_t)len, 0) != len)
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
void do_service()
{
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_storage from_sock_addr;
    int acc_sock;
    socklen_t addr_len;

    for (;;)
    {
        addr_len = sizeof(from_sock_addr);
        if ((acc_sock = accept(sock, (struct sockaddr *)&from_sock_addr, &addr_len)) == -1)
        {
            perror("accept() failed.");
            continue;
        }
        else
        {
            getnameinfo((struct sockaddr *)&from_sock_addr, addr_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);

            fprintf(stderr, "port is %s\n", sbuf);
            fprintf(stderr, "host is %s\n", hbuf);

            do_concrete_service(acc_sock);

            close(acc_sock);
        }
    }
}
void close_server()
{
    close(sock);
    sock = 0;
}
