
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <regex.h>

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
  regex_t line_pattern1;
  int r = regcomp(&line_pattern1, "^0[[:digit:]]+$", REG_EXTENDED);
  if(r != 0)
  {
    size_t len = regerror(r, &line_pattern1, NULL, 0);
    char *errstr = malloc(len);
    regerror(r, &line_pattern1, errstr, len);
    printf("%s\n", errstr);
    free(errstr);
    return EXIT_FAILURE;
  }
  regmatch_t match[16];
  r = regexec(&line_pattern1, "0120333906", 16, match, 0);
  if(r != 0)
  {
    size_t len = regerror(r, &line_pattern1, NULL, 0);
    char *errstr = malloc(len);
    regerror(r, &line_pattern1, errstr, len);
    printf("%s\n", errstr);
    free(errstr);
    regfree(&line_pattern1);
    return EXIT_FAILURE;
  }
  printf("%d, %d\n", match->rm_so, match->rm_eo);
  regfree(&line_pattern1);
  return EXIT_SUCCESS;
}
