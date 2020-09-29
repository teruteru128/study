
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

int main(int argc, char **argv)
{
    int sock;
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    char *msg = NULL;
    ssize_t r = 0;
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int rc = getaddrinfo("localhost", "12345", &hints, &res);
    if(rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    for(ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sock != -1)
        {
            break;
        }
    }

    if (sock == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    if (argc >= 2)
    {
        msg = argv[1];
    }
    else
    {
        msg = "HELLO";
    }

    r = sendto(sock, msg, strlen(msg), 0, ptr->ai_addr, ptr->ai_addrlen);

    printf("%lu\n", r);

    close(sock);
    freeaddrinfo(res);

    return 0;
}
