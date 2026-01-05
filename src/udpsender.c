
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <byteswap.h>

int main(int argc, const char *argv[])
{
    int sock;
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    ssize_t r = 0;
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int rc = getaddrinfo("localhost", "12345", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1)
        {
            break;
        }
    }

    if (sock == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    const char *msg = (argc >= 2) ? argv[1] : "HELLO";
    if (msg == NULL)
    {
        perror("strdup error");
        return EXIT_FAILURE;
    }

    size_t n = strlen(msg);
    r = sendto(sock, msg, n, 0, ptr->ai_addr, ptr->ai_addrlen);

    printf("(%d)%zu, %zd, %s\n", ptr->ai_family == AF_INET ? 4 : (ptr->ai_family == AF_INET6 ? 6 : -1), n, r, (n == r) ? "OK": "NG");

    close(sock);
    freeaddrinfo(res);

    return 0;
}
