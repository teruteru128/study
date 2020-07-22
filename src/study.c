
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define SERVER_NAME "突うずるっ.com"
#define SERVER_PORT "http"
#include <print_addrinfo.h>

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
 */
int main(int argc, char *argv[])
{
  /* NTPサーバのアドレス情報 */
  struct addrinfo hints, *res = NULL, *ptr = NULL;
  //memset(&hints, 0, sizeof(struct addrinfo));
  setlocale(LC_ALL, "ja_JP.UTF-8");
  hints.ai_flags = AI_IDN | AI_CANONIDN | AI_CANONNAME;
  hints.ai_flags = argc >= 3 ? atoi(argv[2]) : (AI_IDN | AI_CANONNAME);
  //hints.ai_flags = AI_IDN | AI_CANONIDN;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = 0;
  hints.ai_protocol = 0;
  int err = getaddrinfo(argc >= 2 ? argv[1] : SERVER_NAME, SERVER_PORT, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo localhost");
    fprintf(stderr, "%s\n", gai_strerror(err));
    return EXIT_FAILURE;
  }
  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    print_addrinfo0(ptr, stderr);
  }
  return EXIT_SUCCESS;
}
