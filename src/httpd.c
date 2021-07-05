
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <print_addrinfo.h>

#define RESPONSE "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nhello world\r\n"
#define MAX_LISTEN_SOCKET_NUM 16
/**
 * @brief シンプルなhttpサーバ
 * @see https://twitter.com/yuta0381/status/1339836231333543936
 * @see https://www.nslabs.jp/socket.rhtml
 * @return int 
 */
int main(void)
{
    struct addrinfo hints, *res, *ptr;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int ecode;
    // AF_UNSPEC + SOCK_STREAM を指定すると 0.0.0.0 と [::] の両方が帰ってくる
    if ((ecode = getaddrinfo(NULL, "4000", &hints, &res)) != 0)
    {
        fprintf(stderr, "failed getaddrinfo() %s\n", gai_strerror(ecode));
        return EXIT_FAILURE;
    }
    int listen_sock = -1;
    int listen_sockets[MAX_LISTEN_SOCKET_NUM] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int socknum = 0;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        print_addrinfo0(ptr, stderr);
        listen_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (listen_sock < 0)
        {
            fprintf(stderr, "socket\n");
            continue;
        }
        if (ptr->ai_family == AF_INET6)
        {
            int on = 1;
            if (setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0)
                return -1;
            else
                printf("set IPV6_V6ONLY\n");
        }
        if (ptr->ai_family == AF_INET || ptr->ai_family == AF_INET6)
        {
            int on = 1;
            if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
                return -1;
            else
                printf("set SO_REUSEADDR\n");
        }

        if (bind(listen_sock, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            fprintf(stderr, "bind, %d: %s\n", listen_sock, strerror(errno));
            close(listen_sock);
            listen_sock = -1;
            continue;
        }

        if (listen(listen_sock, SOMAXCONN) < 0)
        {
            fprintf(stderr, "listen\n");
            close(listen_sock);
            listen_sock = -1;
            continue;
        }
        listen_sockets[socknum++] = listen_sock;
        if (socknum >= MAX_LISTEN_SOCKET_NUM)
            break;
    }
    freeaddrinfo(res);
    if (socknum == 0)
    {
        perror("socknum == 0");
        return EXIT_FAILURE;
    }
    int conn_sock = -1;
    struct sockaddr_storage from_sock_addr;
    socklen_t addr_len = sizeof(from_sock_addr);
    char buf[BUFSIZ];
    ssize_t slen = 0;
    while (1)
    {
        conn_sock = accept(listen_sockets[0], (struct sockaddr *)&from_sock_addr, &addr_len);
        if (conn_sock == -1)
        {
            perror("accept");
            close(conn_sock);
            close(listen_sockets[0]);
            exit(EXIT_FAILURE);
        }
        if (fork() == 0)
        {
            slen = read(conn_sock, buf, BUFSIZ);
            if (slen < 0)
            {
                perror("read");
                close(conn_sock);
                close(listen_sockets[0]);
                exit(EXIT_FAILURE);
            }
            slen = write(conn_sock, RESPONSE, strlen(RESPONSE));
            if (slen < 0)
            {
                perror("read");
                close(conn_sock);
                close(listen_sockets[0]);
                exit(EXIT_FAILURE);
            }
            close(conn_sock);
            close(listen_sockets[0]);
            exit(0);
        }
        close(conn_sock);
    }
    return EXIT_SUCCESS;
}
