
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <stdint.h>
#include <malloc.h> // malloc_usable_size
#include <regex.h>
#include "gettextsample.h"
#include "printint.h"
#include "random.h"
#include "bitset.h"
#include "orz.h"
#include <java_random.h>

#define ISEED 74803317123181L

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
 * https://youtu.be/MCzo6ZMfKa4
 * ターミュレーター
 * 
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 */
int main(int argc, char *argv[])
{
  /*
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Help me!\n"));
  orz(1);
  */
  int64_t seed = initialScramble(ISEED);
  seed = nInverse(seed);
  printf("%ld\n", initialScramble(seed));
  seed = nInverse(seed);
  printf("%ld\n", initialScramble(seed));
  seed = nInverse(seed);
  printf("%ld\n", initialScramble(seed));
  seed = nInverse(seed);
  printf("%ld\n", initialScramble(seed));
  seed = nInverse(seed);
  printf("%ld\n", initialScramble(seed));

  printf("--\n");
  seed = initialScramble(ISEED);
  seed = n(seed);
  printf("%06lx\n", seed);
  seed = n(seed);
  printf("%06lx\n", seed);
  printf("--\n");
  seed = initialScramble(ISEED);
  uint64_t r = nextLong(&seed);
  printf("%06lx\n", r);
  printf("--\n");
  int64_t inv1, inv2, inseed;
  regex_t regex;
  regcomp(&regex, "^....0000", REG_EXTENDED | REG_NEWLINE | REG_ICASE);
  regmatch_t match;
  char buf[BUFSIZ];
  int ret = 0;
  for(seed = 0xffffffff0000L; seed < 0x1000000000000L; seed++)
  {
    inv1 = nInverse(seed);
    inv2 = nInverse(inv1);
    inseed = initialScramble(inv2);
    snprintf(buf, BUFSIZ,"%012lx, %012lx, %ld", inv1, inv2, inseed);
    ret = regexec(&regex, buf, 1, &match, 0);
    if(ret == 0)
    {
      printf("%s\n", buf);
    }
  }

  regfree(&regex);
  return EXIT_SUCCESS;
}
