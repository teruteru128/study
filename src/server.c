
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "server.h"
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/random.h>
#include <unistd.h>

void *work(void *arg)
{
    (void)arg;
    return NULL;
}

int main(int argc, char const *argv[])
{
    struct addrinfo hints, *res = NULL, *ptr = NULL;

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    if (getaddrinfo(NULL, "28152", &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }
    int listen_socket = -1;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        listen_socket
            = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (listen_socket == -1)
            continue;
        if (bind(listen_socket, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            fprintf(stderr, "bind, %d: %s\n", listen_socket, strerror(errno));
            close(listen_socket);
            listen_socket = -1;
            continue;
        }
        if (listen(listen_socket, SOMAXCONN) < 0)
        {
            fprintf(stderr, "listen\n");
            close(listen_socket);
            listen_socket = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    if (listen_socket == -1)
    {
        perror("listen failed");
        return 1;
    }
    int conn_sock = -1;
    struct sockaddr_storage from_sock_addr = { 0 };
    socklen_t addr_len = sizeof(from_sock_addr);
    unsigned char buf[BUFSIZ];
    size_t totalwritesiz = 0;
    size_t writebufsiz = 0;
    unsigned char *writebuf = NULL;
    ssize_t datalen = 0;
    size_t count = 0;
    while (1)
    {
        addr_len = sizeof(from_sock_addr);
        conn_sock = accept(listen_socket, (struct sockaddr *)&from_sock_addr,
                           &addr_len);
        if (conn_sock == -1)
        {
            perror("accept");
            goto finish;
        }
        datalen = read(conn_sock, buf, BUFSIZ);
        if (datalen > 0)
        {
            switch (*(uint64_t *)buf)
            {
            case 1:
                // ゼロ埋めデータ
                totalwritesiz = *(uint64_t *)(buf + 8);
                writebufsiz = datalen >= 24 ? be64toh(*(uint64_t *)(buf + 16))
                                            : BUFSIZ;
                writebuf = malloc(writebufsiz);
                memset(writebuf, 0, writebufsiz);
            case 2:
                // ランダム(未初期化)データ
                totalwritesiz = *(uint64_t *)(buf + 8);
                writebufsiz = datalen >= 24 ? be64toh(*(uint64_t *)(buf + 16))
                                            : BUFSIZ;
                writebuf = malloc(writebufsiz);
            case 3:
                // ランダム(生成器)データ
                totalwritesiz = *(uint64_t *)(buf + 8);
                writebufsiz = datalen >= 24 ? be64toh(*(uint64_t *)(buf + 16))
                                            : BUFSIZ;
                writebuf = malloc(writebufsiz);
                getrandom(writebuf, writebufsiz, GRND_NONBLOCK);
            case -1:
            default:
                break;
            }
            if (writebuf != NULL)
            {
                while (totalwritesiz > 0)
                {
                    if (totalwritesiz > writebufsiz)
                    {
                        write(conn_sock, writebuf, writebufsiz);
                        totalwritesiz -= writebufsiz;
                    }
                    else
                    {
                        write(conn_sock, writebuf, totalwritesiz);
                        totalwritesiz = 0;
                    }
                }

                free(writebuf);
                writebuf = NULL;
                writebufsiz = 0;
                totalwritesiz = 0;
            }
            close(conn_sock);
            conn_sock = -1;
        }
        else if (datalen == 0)
        {
            perror("read");
            close(conn_sock);
            conn_sock = -1;
        }
        else
        {
            close(conn_sock);
            goto finish;
        }
    }

finish:
    close(listen_socket);
    return 0;
}
