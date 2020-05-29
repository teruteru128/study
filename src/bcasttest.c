
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/** https://www.geekpage.jp/programming/linux-network/broadcast.php */
int main(int argc, char **argv)
{
    int sock;
    struct sockaddr_in addr;
    char *msg = NULL;
    int yes = 1;
    ssize_t r = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));

    if (argc >= 2)
    {
        msg = argv[1];
    }
    else
    {
        msg = "HELLO";
    }

    r = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof(addr));

    printf("%lu\n", r);

    close(sock);

    return 0;
}
