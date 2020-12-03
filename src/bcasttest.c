
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>

#define HOSTNAME1 "255.255.255.255"
#define HOSTNAME2 "FF02::1"
#define HOSTNAME3 "FF05::1"

/** https://www.geekpage.jp/programming/linux-network/broadcast.php */
int main(int argc, char **argv)
{
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int rc = getaddrinfo(HOSTNAME1, "12345", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }
    ssize_t r = 0;
    int ret = 0;

    int sock = -1;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1)
            break;
    }

    /* 
     * IPv4 マルチキャスト送信
     * IP_MULTICAST_IF
     * IP_MULTICAST_LOOP
     * IPv4 マルチキャスト受信
     * IP_ADD_MEMBERSHIP
     * IP_DROP_MEMBERSHIP
     * IPv6 マルチキャスト送信
     * IPV6_MULTICAST_HOPS
     */
    if (ptr->ai_family == AF_INET)
    {
        int yes = 1;
        int bcval1 = 0, bcval2 = 0;
        socklen_t len1 = 4, len2 = 4;
        ret = getsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&bcval1, &len1);
        printf("ret : %d\n", ret);
        ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&yes, sizeof(yes));
        printf("ret : %d\n", ret);
        ret = getsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&bcval2, &len2);
        printf("ret : %d\n", ret);
        printf("%d(%d) -> %d(%d)\n", bcval1, len1, bcval2, len2);
    }
    else if (ptr->ai_family == AF_INET6)
    {
    }

    char *msg = strdup(argc >= 2 ? argv[1] : "HELLO");
    if (msg == NULL)
    {
        perror("strdup error");
        return EXIT_FAILURE;
    }

    int f = connect(sock, ptr->ai_addr, ptr->ai_addrlen);
    if (f < 0)
    {
        int err = errno;
        fprintf(stderr, "connect : %d, %s\n", err, strerror(err));
        //perror("connect");
        close(sock);
        return EXIT_FAILURE;
    }

    r = write(sock, msg, strlen(msg));
    if (r < 0)
    {
        perror("write");
        close(sock);
        return EXIT_FAILURE;
    }
    free(msg);

    printf("send : %zd\n", r);

    ret = close(sock);

    if (ret != 0)
    {
        perror("close");
    }
    freeaddrinfo(res);

    return ret;
}
