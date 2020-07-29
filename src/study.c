
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <dirent.h>
#include <string.h>
#include <java_random.h>
#include <time.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/types.h>
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <iconv.h>

#define SERVER_PORT "6910"
// C-implemented p2p earthquake
#define PEER_NAME "cp2peq"
#define PEER_VERSION "0.0.1-alpha"
#define SRC "551 5 ABCDEFG:2005/03/27 12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6,仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

struct args
{
};
struct config
{
};
int parseargs(struct args *args, int argc, char **argv);
void loadconfig();
void startdaemon();
void startserver();
void joinp2pnetwork();
void connecttopeer();

/**
 * --version
 * --help
 * 
 * orz
 * 
 * OpenSSL EVP
 * 対称鍵暗号
 * 認証付き暗号
 * エンベロープ暗号化
 * 署名と検証
 *   EVP_DigestSign
 * メッセージダイジェスト
 * 鍵合意(鍵交換)
 * メッセージ認証符号 (OpenSSL 3～)
 *   EVP_MAC_new_ctx
 * 鍵導出関数
 * strpbrk
 *   文字検索関数
 * strsep
 *   トークン分割(空フィールド対応版)
 * versionsort
 * strverscmp
 * alphasort
 * tor geoip file 読み込み関数
 * geoip_load_file
 * 
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 */
int main(int argc, char *argv[])
{
  struct timespec ts;
  int r = clock_gettime(CLOCK_REALTIME, &ts);
  if (r != 0)
  {
    perror("clock_gettime");
    return EXIT_FAILURE;
  }
  int64_t seed = ts.tv_nsec + ts.tv_sec;
  int64_t rnd = initialScramble(seed);
  char *domains[] = {"p2pquake.dyndns.info", "www.p2pquake.net", "p2pquake.dnsalias.net", "p2pquake.ddo.jp"};

  size_t i = 0, j = 0;
  char *swap;
  for (i = 3; i > 0; i--)
  {
    j = nextIntWithBounds(&rnd, i);
    swap = domains[i];
    domains[i] = domains[j];
    domains[j] = swap;
  }

  struct addrinfo hints, *res = NULL, *ptr = NULL;
  //memset(&hints, 0, sizeof(struct addrinfo));

  int err;
  int sock = 0;
  for (i = 0; i < 4; i++)
  {
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
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
    }
    if (ptr == NULL)
    {
      fprintf(stderr, "connect failed : %s, %s\n", strerror(errno), domains[i]);
      close(sock);
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
  ssize_t s = read(sock, readbuf, BUFSIZ);
  printf("%s", readbuf);

  char writebuf[BUFSIZ];
  snprintf(writebuf, BUFSIZ, "131 1 0.34:%s:%s\r\n", PEER_NAME, PEER_VERSION);
  ssize_t w = write(sock, writebuf, strlen(writebuf));

  memset(readbuf, 0, BUFSIZ);
  s = read(sock, readbuf, BUFSIZ);
  printf("%s", readbuf);
  snprintf(writebuf, BUFSIZ, "113 1\r\n");
  w = write(sock, writebuf, strlen(writebuf));
  memset(readbuf, 0, BUFSIZ);
  s = read(sock, readbuf, BUFSIZ);
  printf("%s", readbuf);
  int code;
  int rep;
  int peerid;
  sscanf(readbuf, "%d %d %d\r\n", &code, &rep, &peerid);
  snprintf(writebuf, BUFSIZ, "115 1 %d\r\n", peerid);
  w = write(sock, writebuf, strlen(writebuf));
  memset(readbuf, 0, BUFSIZ);
  s = read(sock, readbuf, BUFSIZ);
  printf("%s", readbuf);
  snprintf(writebuf, BUFSIZ, "119 1\r\n");
  w = write(sock, writebuf, strlen(writebuf));
  memset(readbuf, 0, BUFSIZ);
  s = read(sock, readbuf, BUFSIZ);
  printf("%s", readbuf);
  close(sock);
  return EXIT_SUCCESS;
}
