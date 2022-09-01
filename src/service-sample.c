
#include <inttypes.h>
#include <netdb.h>
#include <printaddrinfo.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"
#include "serverconfig.h"

static volatile sig_atomic_t running = 1;

static void handler(int sig, siginfo_t *info, void *ctx)
{
    running = 0;
    (void)sig;
    (void)info;
    (void)ctx;
}

int create_server_socket(const char *service)
{
    if (service == NULL)
        return -1;
    // socket
    int serversocket = -1;

    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *ptr = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int ret = -1;
    if ((ret = getaddrinfo(NULL, service, &hints, &res)) != 0)
    {
        fprintf(stdout, "%s\n", gai_strerror(ret));
        return EXIT_FAILURE;
    }
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        printaddrinfo(ptr);
    }
    fputs("--\n", stdout);
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        serversocket
            = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (serversocket == -1)
        {
            perror("");
            continue;
        }
        if (bind(serversocket, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            perror("");
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
        break;
    }
    printaddrinfo(ptr);
    freeaddrinfo(res);
    return serversocket;
}

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

    struct sigaction action = { 0 };
    action.sa_sigaction = handler;
    if (sigaction(SIGINT, &action, NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        return EXIT_FAILURE;
    }

    // socket
    int serversocket = create_server_socket("6500");
    if (serversocket == -1)
    {
        return EXIT_FAILURE;
    }
    uint64_t command = 0;
    uint64_t length = 0;
    struct sockaddr_storage from_sock_addr = { 0 };
    socklen_t addr_len = 0;
    char hbuf[NI_MAXHOST]; /* 返されるアドレスを格納する */
    char sbuf[NI_MAXSERV]; /* 返されるポート番号を格納する */
    // TODO: マルチスレッド化
    const size_t writebufsiz = BUFSIZ;
    unsigned char *writebuf = malloc(writebufsiz);
    memset(writebuf, 0, writebufsiz);
    while (running)
    {
        addr_len = sizeof(from_sock_addr);
        int connection = accept(serversocket,
                                (struct sockaddr *)&from_sock_addr, &addr_len);
        if (connection == -1)
        {
            perror("accept");
            continue;
        }
        getnameinfo((struct sockaddr *)&from_sock_addr, addr_len, hbuf,
                    sizeof(hbuf), sbuf, sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV);
        printf("[%s]:%s\n", hbuf, sbuf);
        if (read(connection, &command, sizeof(uint64_t)) < 1)
        {
            close(connection);
            continue;
        }
        command = be64toh(command);
        if (read(connection, &length, sizeof(uint64_t)) < 1)
        {
            close(connection);
            continue;
        }
        length = be64toh(length);
        printf("%" PRIu64 "\n", length);
        uint64_t i = length;
        ssize_t size = 0;
        while (i > 0)
        {
            if (i >= writebufsiz)
            {
                // データ送信中に割り込み食らったら接続切れるんかな？
                size = write(connection, writebuf, writebufsiz);
                if (size < 0)
                {
                    perror("write 1");
                    break;
                }
                i -= size;
            }
            else
            {
                size = write(connection, writebuf, i);
                if (size < 0)
                {
                    perror("write 2");
                    fprintf(stderr, "write 2: %zd\n", size);
                    break;
                }
                i -= size;
            }
        }
        close(connection);
    }
    close(serversocket);
    free(writebuf);
    return EXIT_SUCCESS;
}
