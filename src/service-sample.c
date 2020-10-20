
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

#include "server.h"

static void usage(int status)
{
    fprintf(stderr, "argument count mismatch error.\nplease input a service name or port number.\n");
    exit(status);
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
    }
    if (init_server(argv[1]) == -1)
    {
        fprintf(stderr, "init_server failure.\n");
        exit(EXIT_FAILURE);
    }
    do_service(NULL);
    close_server();
    return 0;
}
