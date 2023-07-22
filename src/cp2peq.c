
#if !defined _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include <dirent.h>
#include <errno.h>
#include <iconv.h>
#include <java_random.h>
#include <locale.h>
#include <netdb.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PROOF_KEY                                                      \
    "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQC8p/vth2yb/k9x2/PcXKdb6oI3gAbhvr/" \
    "HPTOwla5tQHB83LXNF4Y+Sv/Mu4Uu0tKWz02FrLgA5cuJZfba9QNULTZLTNUgUXIB0m/"    \
    "dq5Rx17IyCfLQ2XngmfFkfnRdRSK7kGnIXvO2/"                                  \
    "LOKD50JsTf2vz0RQIdw6cEmdl+Aga7i8QIBEQ=="
#define PEER_PROOF_KEY                                                        \
    "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDTJKLLO7wjCHz80kpnisqcPDQvA9voNY5" \
    "QuAA+bOWeqvl4gmPSiylzQZzldS+n/M5p4o1PRS24WAO+kPBHCf4ETAns8M02MFwxH/"     \
    "FlQnbvMfi9zutJkQAu3Hq4293rHz+iCQW/MWYB5IfzFBnWtEdjkhqHsGy6sZMMe+qx/"     \
    "F1rcQIBEQ=="
#define SERVERS                                                               \
    "p2pquake.ddo.jp:6910,www.p2pquake.net:6910,p2pquake.info:6910,p2pquake." \
    "xyz:6910"

#define SERVER_PORT "6910"
// C-implemented p2p earthquake
#define PROTOCOL_VERSION "0.34"
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
static const char server_domain_list[4][24]
    = { "p2pquake.dyndns.info", "www.p2pquake.net", "p2pquake.dnsalias.net",
        "p2pquake.ddo.jp" };

int notifyProtocolVersion(int sock)
{
    ssize_t w = send(sock, "113 1\r\n", strlen("113 1\r\n"), 0);
    if (w < 0)
    {
        perror("send 2");
        close(sock);
        return 1;
    }
    return 0;
}

int connect_network(void)
{
    struct timespec resp;
    struct timespec ts;
    clock_getres(CLOCK_REALTIME, &resp);
    clock_gettime(CLOCK_REALTIME, &ts);
    // select server
    int64_t seed = ts.tv_nsec + ts.tv_sec;
    int64_t rnd = initialScramble(seed);
    struct drand48_data data = { 0 };
    unsigned short buf[3];
    buf[0] = (unsigned short)rnd;
    buf[1] = (unsigned short)(rnd >> 16);
    buf[2] = (unsigned short)(rnd >> 32);
    seed48_r(buf, &data);
    const char *domains[4];
    for (int i = 0; i < 4; i++)
    {
        domains[i] = server_domain_list[i];
    }

    // シャッフル
    int i = 0, j = 0;
    const char *swap;
    for (i = 3; i > 0; i--)
    {
        j = nextIntWithBounds(&data, i);
        swap = domains[i];
        domains[i] = domains[j];
        domains[j] = swap;
    }

    // connect server
    struct addrinfo hints = { 0 }, *res = NULL, *ptr = NULL;
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
            sock = -1;
            continue;
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
    ssize_t s = recv(sock, readbuf, BUFSIZ, 0);
    printf("%s", readbuf);
    if (s < 0)
    {
        perror("recv 1");
        close(sock);
        return 1;
    }

    char writebuf[BUFSIZ] = "";
    size_t writelen
        = (size_t)snprintf(writebuf, BUFSIZ, "131 1 %s:%s:%s\r\n",
                           PROTOCOL_VERSION, PEER_NAME, PEER_VERSION);
    ssize_t w = send(sock, writebuf, writelen, 0);
    printf("%s", readbuf);
    if (w < 0)
    {
        perror("send 1");
        close(sock);
        return 1;
    }

    memset(readbuf, 0, BUFSIZ);
    s = recv(sock, readbuf, BUFSIZ, 0);
    printf("%s", readbuf);
    if (s < 0)
    {
        perror("recv 2");
        close(sock);
        return 1;
    }

    if (notifyProtocolVersion(sock))
    {
        return 1;
    }

    memset(readbuf, 0, BUFSIZ);
    s = recv(sock, readbuf, BUFSIZ, 0);
    printf("%s", readbuf);
    if (s < 0)
    {
        perror("recv 3");
        close(sock);
        return 1;
    }

    int code;
    int rep;
    int peerid;

    sscanf(readbuf, "%d %d %d\r\n", &code, &rep, &peerid);

    writelen = (size_t)snprintf(writebuf, BUFSIZ, "115 1 %d\r\n", peerid);
    w = send(sock, writebuf, writelen, 0);
    if (w < 0)
    {
        perror("send 3");
        close(sock);
        return 1;
    }

    memset(readbuf, 0, BUFSIZ);
    s = recv(sock, readbuf, BUFSIZ, 0);
    printf("%s", readbuf);
    if (s < 0)
    {
        perror("recv 4");
        close(sock);
        return 1;
    }

    writelen = (size_t)snprintf(writebuf, BUFSIZ, "119 1\r\n");
    w = send(sock, writebuf, writelen, 0);
    if (w < 0)
    {
        perror("send 4");
        close(sock);
        return 1;
    }

    memset(readbuf, 0, BUFSIZ);
    s = recv(sock, readbuf, BUFSIZ, 0);
    printf("%s", readbuf);
    if (s < 0)
    {
        perror("recv 5");
        close(sock);
        return 1;
    }
    close(sock);
    return EXIT_SUCCESS;
}

/**
 * @brief ネットワーク接続
 *
 * --version
 * --help
 *
 * https://p2pquake.github.io/epsp-specifications/epsp-specifications.html#base-2
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    connect_network();
    // join_network();
    //   問題になるとすればjoin_network()の中でピアに接続する必要があることか？
    /*
    ↓ 211 1
    if(issupportedversion){
        ↑ 131 1 0.20:P2PQ_Client:Beta2_Rev1000
        ↓ 212 1 0.20:P2PQ_Server:Beta2_Rev1000
    }else{
        （↓ 292 1 Protocol_version_incompatible）
        （↑ 192 1）
    }
    ↑ 113 1
    ↓ 233 1 25
    if(issupportlisten){
        ↑ 114 1 25:6911
        ↓ 234 1 1
    }
    ↑ 115 1 25
    ↓ 235 1 192.168.0.1,6911,15:192.168.0.2,6915,81:192.168.0.3,6913,66
    ↑ 155 1 15:66
    ↑ 116 1 25:6911:901:2:6:5,8,3
    ↓ 236 1 29
    if(support rsa){
        ↑ 117 1 25
        if(key was allocated successfully){
            ↓ 237 1 ABCDEFGHIJKLMNOPQRSTU:ABCDEFGHIJKLMN:2005/03/14
        12-34-56:ABCDEFGHIJKLMN }else{ ↓ 295 1 Key_has_allocated
        }
    }
    ↑127 1
    ↓247 1 001,0;002,2;003,5;004,3
    ↑ 118 1
    ↓ 238 1 2005/03/19 12-34-56
    ↑ 119 1
    ↓ 239 1
    */
    // start_server();
    // part_network();
    // 定期エコースレッド
    // コードを見て処理を振り分けるってどうしたらいいのよ？
    // コードの文字列と関数の連想配列的な？
    //
    return EXIT_SUCCESS;
}
