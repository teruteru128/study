
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
#include <openssl/evp.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <netdb.h>

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
 * eventfd
 * timerfd
 * signalfd
 * 
 * Linuxにおけるウェイト処理
 * - sleep
 * - usleep
 * - nanosleep
 * - selectのタイムアウト指定
 * - settimer
 * - timerfd
 * - pthread_cond_timedwait
 * 
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 */
int main(int argc, char *argv[])
{
  struct timespec cur;
  clock_gettime(CLOCK_REALTIME, &cur);
  struct itimerspec spec;
  spec.it_value.tv_sec = cur.tv_sec + 3;
  spec.it_value.tv_nsec = cur.tv_nsec;
  spec.it_interval.tv_sec = 1;
  spec.it_interval.tv_nsec = 0;

  int timer = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
  if(timer < 0)
  {
    perror("timerfd_create");
    return EXIT_FAILURE;
  }
  int ret = timerfd_settime(timer, TFD_TIMER_ABSTIME, &spec, NULL);
  if(ret != 0)
  {
    perror("timerfd_settime");
    close(timer);
    return EXIT_FAILURE;
  }

  uint64_t numexpire = 0;
  // recvで読み込むと失敗を返す
  ssize_t r = read(timer, &numexpire, 8);
  ret = 0;
  if(r < 0)
  {
    perror("recv");
    ret = EXIT_FAILURE;
  }

  printf("%" PRId64 "\n", numexpire);
  close(timer);
  fputs("ひぐらしの\x1b[31mな\x1b[mく頃に\n", stdout);
  return ret;
}
