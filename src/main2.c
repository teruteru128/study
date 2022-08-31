
#define _GNU_SOURCE

#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * 秘密鍵かな？
 * ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc=
 * cm2E2vmE0Nd8aVP/4Ph2S1R6C5bkC1H7CiUBzbcDG3U=
 * BixgbLYk35GP+XHYdK/DgSWIUXyCTwCwEtY4h/G22dw=
 * BH4RDmdo0Yq0Ftiw0lm9ej5BmpZ35kEw2kaWZlZ0Do8=
 * lMhxDh6RPpWOsnJMeS12pTJ/j7EPn+ugpdbNQCGbiwc=
 * 9hZn+KDlwjgrbyFpaX5ibuaO4QfaFbIL79NUrwJlcRQ=
 * T+tDF4I700WFkFhGieYxWgQKPO/MDcntDYzMrqQSZjzwV2DzaI1OM/CsJWE30WBqMI1SxbEQHufR1A76I7ayWN==
 * nySkaCQxGErccmiqLznSQduXgFICpjnl2bo7n3FAhQMlku79plIeL85/etpN865GAnlUpErSppEYHvn4couGh3==
 * ns2bQQ4zlnfcCTSAxEH3gDDYHcBswKw92jQeEgm+9tse74XdX+LNwgfw7OsMUjOGtLMb7R/kXNRXYv1AHi71iV==
 * NxhJ5JwWhUtUccCfJNtVqzdpCMGOaAtknmcEKLyglZFNXE66EiFi9wPFekwekx3ln8m9v5wnfv7V8jSrpZ/SHQ==
 * +3n5qDbtpicXBy+Yyol/TJkg2IoQ01vZ/U2SvgpP+Fdm4DrIYngY7X0ZS53rc/KKIHT//jVqNwNBz1sGFyYUDg==
 * cLtHGFI7X/Xl6Ly03DczMzl2bsHJmI2BMQKKCckUek5vTIiltDPfT3PxdT6zxW1LzwVqJIsQEkxxPNTswgpSFg==
 * pMQBNF+F12AXT3T0mQq7S0l1VcCr/Dw2Q54zeuHH0/1ExLgbhHEsmAHf3WR9nK/Ku1Mc/eU3vaAO78yplJB76A==
 * QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQQ==
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 */
int hiho(int argc, char **argv, const char **envp)
{
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    int rc = getaddrinfo(NULL, "0", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }
    int listen_socket = -1;
    int family = 0;
    int protocol = 0;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((listen_socket
             = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol))
            == -1)
            continue;
        if (bind(listen_socket, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            fprintf(stderr, "bind, %d: %s\n", listen_socket, strerror(errno));
            close(listen_socket);
            listen_socket = -1;
            continue;
        }
        if (listen(listen_socket, SOMAXCONN) < 0)
        {
            fprintf(stderr, "listen\n");
            close(listen_socket);
            listen_socket = -1;
            continue;
        }

        printf("bind OK");
        family = ptr->ai_family;
        protocol = ptr->ai_protocol;
        break;
    }
    printf("%p, listen_socket = %d\n", ptr, listen_socket);
    if (listen_socket == -1)
    {
        perror("");
        return EXIT_FAILURE;
    }
    struct sockaddr_storage storage;
    socklen_t storage_size = sizeof(struct sockaddr_storage);
    rc = getsockname(listen_socket, (struct sockaddr *)&storage,
                     &storage_size);
    if (rc != 0)
    {
        perror("getsockname");
        return EXIT_FAILURE;
    }
    close(listen_socket);
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    rc = getnameinfo((struct sockaddr *)&storage, storage_size, host,
                     NI_MAXHOST, port, NI_MAXSERV,
                     NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0)
    {
        perror("getnameinfo");
        return EXIT_FAILURE;
    }
    printf("%s %s %d %d\n", host, port, family, protocol);
    freeaddrinfo(res);

    return 0;
}
