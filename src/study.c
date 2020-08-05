
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <regex.h>

/* http://tdual.hatenablog.com/entry/2018/05/02/113110 */
struct searchSpace
{
    int64_t seed_start;
    int64_t seed_end;
    int32_t input_x;
    int32_t input_z;
    int32_t kernel_x;
    int32_t kernel_z;
    int32_t threshold;
};

void *searchTask(void *arg)
{
  // 0~624
  // -625 ~ -1
  size_t index = (z + 625) * 1250 + x + 625;
  /*
   * 1250z + 1250 * 625 + x + 625
   * = x + 625 (2z + 1251)
   */
  size_t word = index >> 6;
  size_t bitindex = index & 0x3f;
  uint64_t set[24415];
  (set[word] >> bitindex) & 0x01;
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
