
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>

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
  if(argc < 2)
    return 1;
  
  FILE *in = fopen(argv[1], "r");

  int64_t seed;
  int32_t x;
  int32_t z;
  int32_t sc;
  int32_t chunk;
  int r;
  while((r = fscanf(in, "'%ld, %d, %d, %d/%d\n", &seed, &x, &z, &sc, &chunk)) == 5)
  {
    printf("%ld, %d, %d, %d/%d\n", seed & 0xffffffffffffL, x, z, sc, chunk);
  }

  return EXIT_SUCCESS;
}
