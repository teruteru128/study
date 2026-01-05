
#define _DEFAULT_SOURCE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int rc = getaddrinfo(NULL, "12345", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    int sock = -1;
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

    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    rc = getnameinfo(ptr->ai_addr, ptr->ai_addrlen, host, NI_MAXHOST, port,
                     NI_MAXSERV, 0);
    if (rc != 0)
    {
        perror("getnameinfo");
        freeaddrinfo(res);
        close(sock);
        return EXIT_FAILURE;
    }
    if (ptr->ai_family == AF_INET)
    {
        printf("%s:%s\n", host, port);
    }
    else if (ptr->ai_family == AF_INET6)
    {
        printf("[%s]:%s\n", host, port);
    }

    int r = bind(sock, ptr->ai_addr, ptr->ai_addrlen);
    freeaddrinfo(res);

    if (r == -1)
    {
        perror("bind");
        close(sock);
        return EXIT_FAILURE;
    }

    char buf[BUFSIZ];
    struct sockaddr_storage resaddr;
    socklen_t addr_len;
    do
    {
        addr_len = sizeof(struct sockaddr_storage);
        memset(buf, 0, BUFSIZ);
        ssize_t len = recvfrom(sock, buf, BUFSIZ, 0, (struct sockaddr *)&resaddr, &addr_len);
        getnameinfo((struct sockaddr *)&resaddr, addr_len, host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST);

        printf("[%s(%d):%s][%s] : %zd\n", host, resaddr.ss_family == AF_INET ? 4 : (resaddr.ss_family == AF_INET6 ? 6 : -1), port, buf, len);
    } while (strcmp(buf, "end") != 0);
    r = close(sock);

    if (r)
        perror("close");

    return EXIT_SUCCESS;
}
