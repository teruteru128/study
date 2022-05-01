
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"
#include "serverconfig.h"

static volatile int running = 1;

/*
 * オプション
 * //disable-ipv4
 * //enable-ipv6
 *
 * メインスレッド手続き
 * 1. 初期化
 * 2. accept(2)
 * 3. スレッド起動(pthread_create)
 * 1. コマンドライン引数
 * 2. 送受信スレッド立ち上げ
 * 3. listenスレッド立ち上げ
 *
 * スレッド
 * 1. 初期化スレッド
 * 2. acceptして接続を待つスレッド
 * 3. 受け入れた接続の対応をするスレッド群
 * download thread / upload thread
 */
int main(int argc, char *argv[])
{
    // socket
    int serversocket = -1;

    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *ptr = NULL;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    int ret = -1;
    if ((ret = getaddrinfo(NULL, "6500", &hints, &res)) != 0)
    {
        gai_strerror(ret);
        return EXIT_FAILURE;
    }
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        serversocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (serversocket == -1)
        {
            continue;
        }
        if (bind(serversocket, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            close(serversocket);
            serversocket = -1;
            continue;
        }
        if (listen(serversocket, SOMAXCONN) < 0)
        {
            fprintf(stderr, "listen\n");
            close(serversocket);
            serversocket = -1;
            continue;
        }
    }
    freeaddrinfo(res);
    uint64_t command = 0;
    struct sockaddr_storage from_sock_addr = { 0 };
    socklen_t addr_len = sizeof(from_sock_addr);
    // TODO: マルチスレッド化
    while (running)
    {
        int connection = accept(serversocket, (struct sockaddr *)&from_sock_addr, &addr_len);
        if (connection == -1)
        {
            perror("accept");
            continue;
        }
        if (read(connection, &command, sizeof(uint64_t)) < 1)
        {
            close(connection);
            continue;
        }
        uint64_t length = 0;
        if (read(length, &command, sizeof(uint64_t)) < 1)
        {
            close(connection);
            continue;
        }
        unsigned char buf[BUFSIZ] = "";
        uint64_t i = length;
        ssize_t size = 0;
        while (i > 0)
        {
            if (i >= BUFSIZ)
            {
                // データ送信中に割り込み食らったら接続切れるんかな？
                size = write(connection, buf, BUFSIZ);
                if (size < 0)
                {
                    perror("");
                    break;
                }
                i -= size;
            }
            else
            {
                size = write(connection, buf, i);
                if (size < 0)
                {
                    perror("");
                    break;
                }
                i = 0;
            }
        }
        close(connection);
    }
    close(serversocket);
    return EXIT_SUCCESS;
}
