/*
 * cl.c - サーバからの応答を得るだけのクライアント
 * http://www.koutou-software.net/misc/howto-independ-addfamilysock.php
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <netdb.h>
#include <printaddrinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>

int connect_to_server(char *name, char *service)
{
    int sock = -1;

    struct addrinfo hints; /* 取得したいアドレス情報を指示する */
    struct addrinfo *res; /* 取得したアドレス情報が返ってくる */
    struct addrinfo *ptr; /* 接続要求時に使う */
    /* 引数で指定されたアドレス、ポート番号からアドレス情報を得る */
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int rc;
    rc = getaddrinfo(name, service, &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    /* 得られたアドレスすべてに対し接続を行う */
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {

        /*
         * ソケットを生成する。
         */
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock < 0)
        {
            perror("socket()");
            continue;
        }

        /*
         * 接続要求をする。
         */
        rc = connect(sock, ptr->ai_addr, ptr->ai_addrlen);
        if (rc < 0)
        {
            perror("connect()");
            close(sock);
            sock = -1;
            continue;
        }

        break;
    }
    struct sockaddr_storage storage;
    socklen_t socklen;
    socklen = sizeof(storage);
    rc = getsockname(sock, (struct sockaddr *)&storage, &socklen);
    if (rc != 0)
    {
        perror("getsockname");
    }
    char host[NI_MAXHOST]; /* 返されるアドレスを格納する */

    char serv[NI_MAXSERV]; /* 返されるポート番号を格納する */
    rc = getnameinfo((struct sockaddr *)&storage, socklen, host, NI_MAXHOST,
                     serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0)
    {
        perror("getnameinfo");
    }
    else
    {
        printf("%s, %s\n", host, serv);
    }

    /* アドレス情報を表示する */
    printaddrinfo(ptr);
    /* アドレス情報を開放する */
    freeaddrinfo(res);
    return sock;
}

/**
 * スタートアップ
 */
int main(int argc, char *argv[])
{

    /* 引数の数をチェックする */
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s nodename servname <length> [buffer_size]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    size_t readbufsiz = BUFSIZ;
    if (argc >= 5)
    {
        readbufsiz = strtoul(argv[4], NULL, 10);
    }
    unsigned char *buf = malloc(readbufsiz);
    memset(buf, 0, readbufsiz);
    ssize_t len = 0;
    int sock = connect_to_server(argv[1], argv[2]);

    if (sock == -1)
    {
        perror("sock == -1");
        return EXIT_FAILURE;
    }

    uint64_t command = 1;
    len = write(sock, &command, sizeof(uint64_t));
    if (len == 0)
    {
        perror("write 1");
        close(sock);
        return EXIT_FAILURE;
    }
    uint64_t length = 1UL << 20;
    if (argc >= 4)
    {
        length = strtoul(argv[3], NULL, 10);
    }

    uint64_t tmplength = htobe64(length);
    len = write(sock, &tmplength, sizeof(uint64_t));
    if (len == 0)
    {
        perror("write 1");
        close(sock);
        return EXIT_FAILURE;
    }

    size_t sumoflen = length;
    int rc = 0;

    /* サーバからの応答を表示する */
    while (sumoflen > 0)
    {
        len = read(sock, buf, readbufsiz);
        if (len < 0)
        {
            perror("read 1");
            rc = 1;
            break;
        }
        sumoflen -= len;
    }
    free(buf);
    buf = NULL;

    close(sock);

    return rc != 0;
}
