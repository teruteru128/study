
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR "abcdefgabcdefghij"

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
 * strspn
 */
int main(int argc, char *argv[])
{
  int c = 0;
  char *v = NULL;
  for (c = 1; c < argc; c++)
  {
    v = argv[c];
    printf("%s\n", v);
  }
  char search[21];
  size_t p;

  printf("文字群を入力しなさい。\n");
  int r = scanf("%20s", search);
  if (r == EOF)
  {
    perror("scanf");
    return EXIT_FAILURE;
  }

  p = strspn(STR, search);
  printf("%zd\n", p);

  return EXIT_SUCCESS;
}
