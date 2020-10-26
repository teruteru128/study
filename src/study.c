
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
#include <syslog.h>

#define LIMIT 16

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
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  /* 次の実行日時を取得する */
  /* 現在時刻を取得する */
  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  struct tm tm;
  tzset();
  gmtime_r(&currentTime.tv_sec, &tm);
  printf("%d %04d-%02d-%02dT%02d:%02d:%02dZ\n", tm.tm_isdst, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  localtime_r(&currentTime.tv_sec, &tm);
  printf("%d %d-%d-%dT%d:%d:%d+09:00\n", tm.tm_isdst, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef __USE_MISC
  printf("%ld, %s\n", tm.tm_gmtoff, tm.tm_zone);
#endif
  tm.tm_sec = 0;
  tm.tm_min = 10;
  tm.tm_hour = 10;
  tm.tm_mday = 10;
  tm.tm_mon = 9;
  time_t time1 = mktime(&tm);
  tm.tm_sec = 0;
  tm.tm_min = 10;
  tm.tm_hour = 10;
  tm.tm_mday = 10;
  tm.tm_mon = 9;
  tm.tm_year = 2010 - 1900;
  time_t time2 = mktime(&tm);
  printf("%lf\n", difftime(time1, time2));
  syslog(LOG_NOTICE, "HELLO WORLD");
  return EXIT_SUCCESS;
}
