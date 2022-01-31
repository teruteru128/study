
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

struct config;
/**
 * @brief config object
 * 構造体の定義とgetConfig関数のプロトタイプ宣言をヘッダーに置く
 */
struct config
{
};

static struct config singleton = {};
static pthread_once_t config_singleton_init = PTHREAD_ONCE_INIT;

static void config_init_func() {
    // 初期値の設定
    // ファイルからのロード
}

struct config *getConfig()
{
    pthread_once(&config_singleton_init, config_init_func);
    return &singleton;
}

int loadConfigFromFile(struct config *config, char *configfilepath)
{
    return 0;
}

int setupConfigFromCmdArgs(struct config *config, int argc, char **argv)
{
    return 0;
}

static void usage(int status)
{
    fprintf(stderr, "argument count mismatch error.\nplease input a service "
                    "name or port number.\n");
    exit(status);
}

int acceptedsocket = -1;
pthread_mutex_t acceptedsocket_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t acceptedsocket_cond = PTHREAD_COND_INITIALIZER;

void *taskthread(void *a)
{

    int sock = -1;
    unsigned short command = 0;
    size_t length;
    while (running)
    {
        pthread_mutex_lock(&acceptedsocket_mutex);
        while (acceptedsocket == -1)
        {
            pthread_cond_wait(&acceptedsocket_cond, &acceptedsocket_mutex);
        }
        sock = acceptedsocket;
        acceptedsocket = -1;
        pthread_mutex_unlock(&acceptedsocket_mutex);
        read(sock, &command, sizeof(unsigned short));
        command = be16toh(command);

        switch (command)
        {
        case 1:
            read(sock, &length, sizeof(size_t));
            length = be64toh(length);
            break;

        default:
            break;
        }
    }
    return NULL;
}

void *acceptthrad(void *a)
{
    char service[NI_MAXSERV] = "";
    struct addrinfo hints;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo *res;

    getaddrinfo(NULL, service, &hints, &res);

    int sock = -1;

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

    struct sockaddr_storage from_sock_addr = { 0 };
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    int acsock;
    while (running)
    {
        acsock = accept(sock, (struct sockaddr *)&from_sock_addr, &addr_len);
        pthread_mutex_lock(&acceptedsocket_mutex);
        acceptedsocket = acsock;
        pthread_cond_signal(&acceptedsocket_cond);
        pthread_mutex_unlock(&acceptedsocket_mutex);
    }
    return NULL;
}

/*
 * オプション
 * //disable-ipv4
 * //enable-ipv6
 *
 * 手順
 * 1. 初期化
 * 2. accept(2)
 * 3. スレッド起動(pthread_create)
 *
 * スレッド
 * 1. 初期化スレッド
 * 2. acceptして接続を待つスレッド
 * 3. 受け入れた接続の対応をするスレッド群
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(EXIT_FAILURE);
        return 1;
    }
    struct service_arg arg;
    strncpy(arg.port, argv[1], NI_MAXSERV);
    pthread_t acceptthread;
    pthread_create(&acceptthread, NULL, do_service, &arg);
    pthread_t work_threads[16];
    running = 0;
    return 0;
}
