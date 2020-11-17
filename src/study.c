
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <byteswap.h>
#include <inttypes.h>
#include <stddef.h>

#define LIMIT 16

/**
 * --help
 * --verbose
 * --version
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
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  unsigned char buf[20];
  FILE *r = fopen("/dev/urandom", "rb");
  if (r == NULL)
  {
    return EXIT_FAILURE;
  }
  size_t req = 12;
  size_t len = fread(buf, sizeof(char), req, r);
  if (len != req)
  {
    perror("fread error");
  }
  fclose(r);
  *((uint64_t *)(buf + 12)) = le64toh(1);
  errno = 0;
  ptrdiff_t diff = (int *)buf - __errno_location();
  printf("%p %p %016tx\n", __errno_location(), &buf, diff);
  for (size_t i = 0; i < 20; i++)
  {
    printf("%02x", buf[i]);
  }
  fputs("\nひぐらしの\e[31mな\e[mく頃に\n", stdout);
  return EXIT_SUCCESS;
}
