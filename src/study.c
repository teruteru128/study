
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <java_random.h>
#include "gettext.h"
#define _(str) gettext(str)
#include "bouyomi.h"

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
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  long seed = tp.tv_sec + tp.tv_nsec;
  seed = n(seed);
  size_t count = 0;
  while(1)
  {
    if(nextIntWithBounds(&seed, 1000) == 0)
    {
      printf("%lu\n", count);
      count = 0;
    }
    else
    {
      count++;
    }
  }
  return EXIT_SUCCESS;
}
