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
#include <sys/types.h>
#include <unistd.h>

/**
 * スタートアップ
 */
int main(int argc, char *argv[])
{
    struct addrinfo hints; /* 取得したいアドレス情報を指示する */
    struct addrinfo *res; /* 取得したアドレス情報が返ってくる */
    struct addrinfo *ptr; /* 接続要求時に使う */

    int rc;

    /* 引数の数をチェックする */
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s nodename servname <length> [buffer_size]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* 引数で指定されたアドレス、ポート番号からアドレス情報を得る */
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }
    size_t readbufsiz = 1024 * 1024 * 1024;
    unsigned char *buf = malloc(readbufsiz);
    int sock = -1;
    ssize_t len = 0;

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
    /* アドレス情報を表示する */
    printaddrinfo(ptr);
    if (sock == -1)
    {
        freeaddrinfo(res);
        perror("sock == -1");
        return EXIT_FAILURE;
    }
    freeaddrinfo(res);
    res = NULL;

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

    /* サーバからの応答を表示する */
    while (sumoflen > 0)
    {
        if (sumoflen >= readbufsiz)
        {
            len = read(sock, buf, readbufsiz);
            if (len < 0)
            {
                perror("read 1");
                break;
            }
            sumoflen -= len;
        }
        else
        {
            len = read(sock, buf, sumoflen);
            if (len < 0)
            {
                perror("read 2");
                break;
            }
            sumoflen = 0;
        }
    }

    close(sock);

    return EXIT_SUCCESS;
}
