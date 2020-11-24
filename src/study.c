
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef ENABLE_REGEX
#include <regex.h>
#else
#include <string.h>
#endif

/**
 * 
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
 * https://youtu.be/MCzo6ZMfKa4
 * ターミュレーター
 * 
 * regex.h
 * - 最左最短一致
 * - 否定先読み
 * - 強欲な数量子
 * 
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 * 
 * @brief sanbox func.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
#ifdef ENABLE_REGEX
  regex_t pattern1;
  regcomp(&pattern1, "3", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
#endif
  char buf[24];
  for (size_t i = 1; i <= 40; i++)
  {
    snprintf(buf, 24, "%zd", i);
    //ltoa(i, buf, 10);
    if (i % 3 == 0 ||
#ifdef ENABLE_REGEX
        regexec(&pattern1, buf, 0, NULL, 0) == 0
#else
        !strchr(buf, '3')
#endif
    )
    {
      fputs("aho\n", stdout);
    }
    else
    {
      printf("%s\n", buf);
    }
  }
#ifdef ENABLE_REGEX
  regfree(&pattern1);
#endif
  return EXIT_SUCCESS;
}
