
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

/** https://www.geekpage.jp/programming/linux-network/broadcast.php */
int main(int argc, char **argv)
{
    int sock;
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(12345),
        .sin_addr.s_addr = inet_addr("255.255.255.255")
    };
    char *msg = NULL;
    int yes = 1;
    int j = 0, k = 0;
    socklen_t len1 = 4, len2 = 4;
    ssize_t r = 0;
    int ret = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    ret = getsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&j, &len1);
    printf("ret : %d\n", ret);
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&yes, sizeof(yes));
    printf("ret : %d\n", ret);
    ret = getsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&k, &len2);
    printf("ret : %d\n", ret);
    printf("%d(%d) -> %d(%d)\n", j, len1, k, len2);

    if (argc >= 2)
    {
        msg = strdupa(argv[1]);
    }
    else
    {
        msg = strdupa("HELLO");
    }

    r = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof(addr));

    printf("send : %lu\n", r);

    ret = close(sock);

    if (ret != 0)
    {
        perror("close");
    }

    return ret;
}
