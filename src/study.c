
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>

struct domainname
{
  char *dname;
};

int cmp(const void *p1, const void *p2, void *arg)
{
  return strcmp(((struct domainname *)p1)->dname, ((struct domainname *)p2)->dname);
}

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
 */
int main(int argc, char *argv[])
{
  struct domainname domains[4] = {
    {"p2pquake.dyndns.info"},
    {"www.p2pquake.net"},
    {"p2pquake.dnsalias.net"},
    {"p2pquake.ddo.jp"}
  };
  qsort_r(domains, 4UL, sizeof(struct domainname), cmp, NULL);
  int i = 0;
  for(i = 0; i < 4; i++)
  {
    printf("%s\n", domains[i].dname);
  }
  return EXIT_SUCCESS;
}
