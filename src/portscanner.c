
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

#define ip_address "192.168.1.3"

// http://developer.wonderpla.net/entry/blog/engineer/network_program_with_cpp_01/
int main(int argc, char **argv)
{
    struct addrinfo hints, *res, *ptr;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int sock = 0;
    char port_str[12];
    int err = 0;
    for (int port = 1; port < 65536; port++)
    {
        snprintf(port_str, 12, "%u", port);
        if ((err = getaddrinfo(ip_address, port_str, &hints, &res)) != 0)
        {
            perror("getaddrinfo localhost");
            fprintf(stderr, "%s\n", gai_strerror(err));
            continue;
        }
        for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
        {
            sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (sock == -1)
                continue;

            break;
        }
        if (ptr == NULL)
        {
            perror("bind failed");
            close(sock);
            continue;
        }
        printf("%5d : ", port);

        /* サーバにコネクトを行う。この関数はブロック型である。
       　 コネクトのタイムアウトはデバイスドライバーの実装によって違う。
       このタイムアウト値の設定もくせものでOSによっては設定ができない。
       なので、タイムアウトをさせる場合は、別途実装する必要があるが、色々と説明が長くなるので
       ライブラリにまとめる際のコードを参考にして欲しい。 

       第二引数のstruct sockaddrのキャストだが、sockの第一引数に何を指定しているかで指定される構造体が違うためにキャストすることになる。
       現代のソフトウェア工学的には気持ち悪い実装かも知れないが、connectがシステムコールである故にやむを得ないところがある。
       */
        if (connect(sock, ptr->ai_addr, ptr->ai_addrlen) != 0)
        {
            /* 失敗*/
            printf("socket connect error!!! (%u, %d)\n", port, errno);
            //return EXIT_FAILURE;
        }
        else
        {
            printf("OK(%u)\n", port);
        }
        close(sock);
    }
    return EXIT_SUCCESS;
}
