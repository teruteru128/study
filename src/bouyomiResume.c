
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sendresume(char *name, char *service)
{
    //送信するデータの生成
    unsigned char header[2];
    *((short *)(header + 0)) = (short)htole16((unsigned short)0x0020);

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(name, service, &hints, &res);
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    send(sock, header, 2, 0);

    close(sock);

    return 0;
}

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

    char name[NI_MAXHOST] = "192.168.12.5";
    char service[NI_MAXSERV] = "50001";

    if (sendresume(name, service) != 0)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
