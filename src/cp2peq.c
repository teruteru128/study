
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include <java_random.h>
#include <dirent.h>
#include <locale.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <iconv.h>
#include <unistd.h>

#define SERVER_PORT "6910"
// C-implemented p2p earthquake
#define PEER_NAME "cp2peq"
#define PEER_VERSION "0.0.1-alpha"
#define SRC                                                                        \
    "551 5 ABCDEFG:2005/03/27 "                                                    \
    "12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6," \
    "仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

struct args
{
    long padding;
};
struct config
{
    long padding;
};
int parseargs(struct args *args, int argc, char **argv);
void loadconfig();
void startdaemon();
void startserver();
void joinp2pnetwork();
void connecttopeer();

int connect_network(struct timespec *resp, struct timespec *ts)
{
    // select server
    int64_t seed = ts->tv_nsec + ts->tv_sec;
    int64_t rnd = initialScramble(seed);
    char domainsin[4][24] = { "p2pquake.dyndns.info", "www.p2pquake.net",
                              "p2pquake.dnsalias.net", "p2pquake.ddo.jp" };
    char *domains[4];
    for (int i = 0; i < 4; i++)
    {
        domains[i] = domainsin[i];
    }

    int i = 0, j = 0;
    char *swap;
    for (i = 3; i > 0; i--)
    {
        j = nextIntWithBounds(&rnd, i);
        swap = domains[i];
        domains[i] = domains[j];
        domains[j] = swap;
    }

    // connect server
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    // memset(&hints, 0, sizeof(struct addrinfo));

    int err;
    int sock = -1;
    for (i = 0; i < 4; i++)
    {
        err = getaddrinfo(domains[i], SERVER_PORT, &hints, &res);
        if (err != 0)
        {
            perror("getaddrinfo");
            fprintf(stderr, "%d, %s\n", err, gai_strerror(err));
            continue;
        }
        for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
        {
            sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (sock == -1)
                continue;

            if (connect(sock, ptr->ai_addr, ptr->ai_addrlen) != -1)
                break;

            close(sock);
            sock = -1;
        }
        if (ptr == NULL)
        {
            fprintf(stderr, "connect failed : %s, %s\n", strerror(errno),
                    domains[i]);
            close(sock);
            continue;
            sock = -1;
        }
        break;
    }
    if (ptr == NULL)
    {
        perror("connect failed");
        return EXIT_FAILURE;
    }
    printf("%s\n", domains[i]);
    char readbuf[BUFSIZ];
    ssize_t s = read(sock, readbuf, BUFSIZ);
    printf("%s", readbuf);

    char writebuf[BUFSIZ];
    size_t writelen = (size_t)snprintf(
        writebuf, BUFSIZ, "131 1 0.34:%s:%s\r\n", PEER_NAME, PEER_VERSION);
    ssize_t w = write(sock, writebuf, writelen);

    memset(readbuf, 0, BUFSIZ);
    s = read(sock, readbuf, BUFSIZ);
    printf("%s", readbuf);
    writelen = (size_t)snprintf(writebuf, BUFSIZ, "113 1\r\n");
    w = write(sock, writebuf, writelen);
    memset(readbuf, 0, BUFSIZ);
    s = read(sock, readbuf, BUFSIZ);
    printf("%s", readbuf);
    int code;
    int rep;
    int peerid;
    sscanf(readbuf, "%d %d %d\r\n", &code, &rep, &peerid);
    writelen = (size_t)snprintf(writebuf, BUFSIZ, "115 1 %d\r\n", peerid);
    w = write(sock, writebuf, writelen);
    memset(readbuf, 0, BUFSIZ);
    s = read(sock, readbuf, BUFSIZ);
    printf("%s", readbuf);
    writelen = (size_t)snprintf(writebuf, BUFSIZ, "119 1\r\n");
    w = write(sock, writebuf, writelen);
    memset(readbuf, 0, BUFSIZ);
    s = read(sock, readbuf, BUFSIZ);
    printf("%s", readbuf);
    close(sock);
    return EXIT_SUCCESS;
}

/**
 * @brief ネットワーク接続
 *
 * --version
 * --help
 *
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    struct timespec res;
    struct timespec ts;
    clock_getres(CLOCK_REALTIME, &res);
    clock_gettime(CLOCK_REALTIME, &ts);
    connect_network(&res, &ts);
    return EXIT_SUCCESS;
}
