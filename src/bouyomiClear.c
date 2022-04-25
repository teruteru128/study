
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    //コマンドライン引数処理
    switch (argc)
    {
    case 1:
        break;
    default:
        printf("使用法1>%s\n", argv[0]);
        return -1;
    }

    //送信するデータの生成
    short header = (short)htole16((unsigned short)0x0040);

    char host[NI_MAXHOST] = "192.168.12.5";
    char port[NI_MAXSERV] = "50001";

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    hints.ai_socktype = SOCK_STREAM;

    int errcode;
    if (errcode = getaddrinfo(host, port, &hints, &res))
    {
        fprintf(stderr, "%s\n", gai_strerror(errcode));
        return EXIT_FAILURE;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1)
    {
        perror("socket");
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }
    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("socket");
        close(sock);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }
    freeaddrinfo(res);
    res = NULL;

    if (send(sock, &header, 2, 0) == -1)
    {
        perror("socket");
        close(sock);
        return EXIT_FAILURE;
    }

    close(sock);

    return EXIT_SUCCESS;
}
