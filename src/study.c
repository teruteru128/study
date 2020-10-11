
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
  localtime_r(&currentTime.tv_sec, &tm);
  tm.tm_sec = 0;
  tm.tm_min = 0;
  tm.tm_hour = 4;
  tm.tm_mday = 25;
  tm.tm_mon = 11;
  struct timespec christmasTime;
  christmasTime.tv_sec = mktime(&tm);
  //christmasTime.tv_nsec = currentTime.tv_nsec;
  christmasTime.tv_nsec = 0;
  double diffsec = difftime(christmasTime.tv_sec, currentTime.tv_sec);
  long diffnsec = christmasTime.tv_nsec - currentTime.tv_nsec;
  if (diffnsec < 0)
  {
    fprintf(stdout, "ああ！%lf, %ld\n", diffsec, diffnsec);
    diffnsec += 1000000000;
    diffsec--;
  }
  if (diffsec < 0)
  {
    // 今年のクリスマスは終了済み
    printf("日本は終了しました＼(^o^)／\n");
    return 0;
  }
  if (diffsec == 0 && diffnsec == 0)
  {
    // 完全一致
    printf("一致しました＼(^o^)／\n");
  }

  printf("%ld.%09ld\n", (long)diffsec, diffnsec);

  {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_lock(&mutex);
    // 実行時間まで待つ
    int sig = pthread_cond_timedwait(&cond, &mutex, &christmasTime);
    if (sig != ETIMEDOUT)
    {
      pthread_mutex_unlock(&mutex);
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&cond);
      return 1;
    }
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    fputs("時間が経過しました\n", stdout);
  }
  return EXIT_SUCCESS;
}
