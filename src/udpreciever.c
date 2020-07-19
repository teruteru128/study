
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int main()
{
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(12345), .sin_addr.s_addr = INADDR_ANY};

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int r = bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    if (r == -1)
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    char buf[2048];
    do
    {
        memset(buf, 0, sizeof(buf));
        ssize_t len = recv(sock, buf, sizeof(buf), 0);

        printf("%s : %ld\n", buf, len);
    } while (strcmp(buf, "end") != 0);
    r = close(sock);

    if(r)
        perror("close");

    return EXIT_SUCCESS;
}
