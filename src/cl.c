/*
 * cl.c - サーバからの応答を得るだけのクライアント
 * http://www.koutou-software.net/misc/howto-independ-addfamilysock.php
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <print_addrinfo.h>

/**
 * スタートアップ
 */
int main(int argc, char *argv[])
{
  struct addrinfo hints;   /* 取得したいアドレス情報を指示する */
  struct addrinfo *res;    /* 取得したアドレス情報が返ってくる */
  struct addrinfo *adrinf; /* 接続要求時に使う */

  int rc;

  /* 引数の数をチェックする */
  if (argc != 3)
  {
    fprintf(stderr, "usage: %s nodename servname\n", argv[0]);
    return EXIT_FAILURE;
  }

  /* 引数で指定されたアドレス、ポート番号からアドレス情報を得る */
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = 0;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  rc = getaddrinfo(argv[1], argv[2], &hints, &res);
  if (rc != 0)
  {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
    return EXIT_FAILURE;
  }

  char buf[BUFSIZ];
  int sock = 0;
  ssize_t len = 0;

  /* 得られたアドレスすべてに対し接続を行う */
  for (adrinf = res; adrinf != NULL; adrinf = adrinf->ai_next)
  {

    /*
     * 接続要求をする。
     */
    sock = socket(adrinf->ai_family, adrinf->ai_socktype, adrinf->ai_protocol);
    if (sock < 0)
    {
      perror("socket()");
      continue;
    }

    rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
    if (rc < 0)
    {
      perror("connect()");
      close(sock);
      continue;
    }

    break;
  }
  /* アドレス情報を表示する */
  print_addrinfo(adrinf);

  len = write(sock, "334", strlen("334"));
  if(len == 0)
  {
    perror("write 1");
    close(sock);
    return EXIT_FAILURE;
  }

  /* サーバからの応答を表示する */
  while (0 < (len = read(sock, buf, sizeof(buf))))
  {
    printf("%ld, %s\n", len, buf);
  }
  len = write(sock, "-1", strlen("-1"));if(len == 0)
  {
    perror("write 2");
    close(sock);
    return EXIT_FAILURE;
  }

  close(sock);

  return EXIT_SUCCESS;
}
