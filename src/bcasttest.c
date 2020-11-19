
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

/** https://www.geekpage.jp/programming/linux-network/broadcast.php */
int main(int argc, char **argv)
{
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int rc = getaddrinfo("255.255.255.255", "12345", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }
    int yes = 1;
    int bcval1 = 0, bcval2 = 0;
    socklen_t len1 = 4, len2 = 4;
    ssize_t r = 0;
    int ret = 0;

    int sock = -1;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1)
            break;
    }

    ret = getsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&bcval1, &len1);
    printf("ret : %d\n", ret);
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&yes, sizeof(yes));
    printf("ret : %d\n", ret);
    ret = getsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&bcval2, &len2);
    printf("ret : %d\n", ret);
    printf("%d(%d) -> %d(%d)\n", bcval1, len1, bcval2, len2);

    char *msg = strdup(argc >= 2 ? argv[1] : "HELLO");
    if(msg == NULL)
    {
        perror("strdup error");
        return EXIT_FAILURE;
    }

    r = sendto(sock, msg, strlen(msg), 0, ptr->ai_addr, ptr->ai_addrlen);
    free(msg);

    printf("send : %lu\n", r);

    ret = close(sock);

    if (ret != 0)
    {
        perror("close");
    }
    freeaddrinfo(res);

    return ret;
}
