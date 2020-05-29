/*
 * cl.c - サーバからの応答を得るだけのクライアント
 * http://www.koutou-software.net/misc/howto-independ-addfamilysock.php
 */

#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include "print_addrinfo.h"

/**
 * スタートアップ
 */
int main(int argc, char *argv[])
{
  struct addrinfo hints;    /* 取得したいアドレス情報を指示する */
  struct addrinfo *res;     /* 取得したアドレス情報が返ってくる */
  struct addrinfo *adrinf;  /* 接続要求時に使う */

  int rc;

  /* 引数の数をチェックする */
  if (argc != 3) {
    fprintf(stderr, "usage: %s nodename servname\n", argv[0]);
    return -1;
  }

  /* 引数で指定されたアドレス、ポート番号からアドレス情報を得る */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;

  rc = getaddrinfo(argv[1], argv[2], &hints, &res);
  if (rc != 0) {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
    return -1;
  }

  /* 得られたアドレスすべてに対し接続を行う */
  for (adrinf = res; adrinf != NULL; adrinf = adrinf->ai_next) {

    char buf[2048];
    int sock;
    int len;

    /*
     * 接続要求をする。
     */
    sock = socket(adrinf->ai_family, adrinf->ai_socktype, adrinf->ai_protocol);
    if (sock < 0) {
      perror("socket()");
      continue;
    }

    rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
    if (rc < 0) {
      perror("connect()");
      close(sock);
      continue;
    }

    /* アドレス情報を表示する */
    print_addrinfo(adrinf);

    /* サーバからの応答を表示する */
    while (0 < (len = read(sock, buf, sizeof(buf)))) {
      printf("%.*s", len, buf);
    }

    close(sock);
  }

  return 0;
}

