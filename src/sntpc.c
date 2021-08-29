
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include "ntp.h"
#include <byteswap.h>
#include <locale.h>
#include <netdb.h>
#include <poll.h>
#include <printaddrinfo.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> //close()

#define SERVER_NAME "ntp.nict.jp"
#define SERVER_PORT "ntp"
/*
https://www.nakka.com/lib/inet/sntpc.html
https://www.nakka.com/lib/inet/sntpcex.html
*/
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    /* ソケット(ディスクリプタ) */
    int recv_sock = 0;

    /* NTPサーバのアドレス情報 */
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    // memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int err = getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &res);
    if (err != 0)
    {
        perror("getaddrinfo localhost");
        fprintf(stderr, "%s\n", gai_strerror(err));
        return EXIT_FAILURE;
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        printaddrinfo0(ptr, stderr);
    }

    fputs("--\n", stderr);

    /* ソケットを作成して接続 */
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        recv_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (recv_sock == -1)
            continue;

        if (connect(recv_sock, ptr->ai_addr, ptr->ai_addrlen) != -1)
            break;

        close(recv_sock);
    }
    if (ptr == NULL)
    {
        perror("connect failed");
        close(recv_sock);
        return EXIT_FAILURE;
    }
    printaddrinfo0(ptr, stderr);
    freeaddrinfo(res);
    res = NULL;
    ptr = NULL;

    struct NTP_Packet sendpacket;
    memset(&sendpacket, 0, sizeof(struct NTP_Packet));
    sendpacket.Control_Word = bswap_32(0x23000000);
    dumpNTPpacket(&sendpacket, stderr);

    ssize_t w = send(recv_sock, &sendpacket, sizeof(struct NTP_Packet), 0);
    if (w == (ssize_t)-1)
    {
        perror("send");
        close(recv_sock);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }
    struct NTP_Packet recvpacket;
    memset(&recvpacket, 0, sizeof(struct NTP_Packet));
    ssize_t r = read(recv_sock, &recvpacket, sizeof(struct NTP_Packet));
    if (r != sizeof(struct NTP_Packet))
    {
        perror("recv");
        close(recv_sock);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }
    freeaddrinfo(res);
    close(recv_sock);
    dumpNTPpacket(&recvpacket, stderr);
    return EXIT_SUCCESS;
}
