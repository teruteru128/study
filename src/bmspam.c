

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>
#include <string.h>
#include <uuid/uuid.h>
#include <bitmessage.h>
#include <base64.h>
#include <bm.h>
#include <bmapi.h>
#include <err.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include "bmspam.h"

void die_if_fault_occurred(xmlrpc_env *env);

#define USER_NAME "teruteru128"
#define PASSWORD "testpassword"

/**
 * int main(int argc, char* argv[])
 * {
 *   // グローバル定数初期化
 *   global_init();
 *   // 設定ファイルパース
 *   configfile_parse();
 *   // コマンドライン引数パース
 *   arg_parse();
 *   // メイン処理
 *   do_main();
 *   // 後片付け
 *   global_cleanup();
 * }
 * TODO: コマンドライン引数でtest.txtとaddressbook.txtを切り替えられるようにする
 * */
int main(int const argc, const char **const argv)
{
  /* 次の実行日時を取得する */
  /* 現在時刻を取得する */
  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  currentTime.tv_nsec = 0;
  // 今年のクリスマスのUNIXタイムスタンプを取得する
  struct tm tm;
  localtime_r(&currentTime.tv_sec, &tm);
  tm.tm_sec = 0;
  tm.tm_min = 0;
  tm.tm_hour = 4;
  tm.tm_mday = 25;
  tm.tm_mon = 11;
  time_t christmasTime = mktime(&tm);
  // 実行判定。現在時刻が今年のクリスマスよりも未来だった場合実行しない
  double diffsec = difftime(christmasTime, currentTime.tv_sec);
  if (diffsec < 0)
  {
    // 今年のクリスマスは終了済み
    printf("日本は終了しました＼(^o^)／\n");
    return 0;
  }

  printf("%.0lf\n", diffsec);

  struct itimerspec detonationTime;
  // クリスマスを有効期限に設定
  detonationTime.it_value.tv_sec = christmasTime;
  detonationTime.it_value.tv_nsec = 0;
  detonationTime.it_interval.tv_sec = 0;
  detonationTime.it_interval.tv_nsec = 0;

  {
    int timer = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timer < 0)
    {
      perror("timerfd_create");
      return EXIT_FAILURE;
    }
    int ret = timerfd_settime(timer, TFD_TIMER_ABSTIME, &detonationTime, NULL);
    if (ret != 0)
    {
      perror("timerfd_settime");
      close(timer);
      return EXIT_FAILURE;
    }
    uint64_t numexpire = 0;
    ssize_t r = read(timer, &numexpire, 8);
    ret = 0;
    if (r < 0)
    {
      perror("recv");
      ret = EXIT_FAILURE;
    }
    close(timer);
  }

  xmlrpc_env env;
  xmlrpc_client *clientP;
  xmlrpc_server_info *serverP;
  xmlrpc_value *resultP;
  /* Initialize our error-handling environment. */
  xmlrpc_env_init(&env);
  die_if_fault_occurred(&env);
  xmlrpc_client_setup_global_const(&env);
  die_if_fault_occurred(&env);
  xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
                       &clientP);
  die_if_fault_occurred(&env);
  serverP = xmlrpc_server_info_new(&env, SERVER_URL);
  die_if_fault_occurred(&env);
  xmlrpc_server_info_set_user(&env, serverP, USER_NAME, PASSWORD);
  die_if_fault_occurred(&env);
  xmlrpc_server_info_allow_auth_basic(&env, serverP);
  die_if_fault_occurred(&env);

  char toaddress[64];
  char *tmp = NULL;
  xmlrpc_value *paramArray = NULL;
  xmlrpc_value *toaddressv = NULL;
  xmlrpc_value *fromaddressv = xmlrpc_string_new(&env, GENELRAL);
  die_if_fault_occurred(&env);
  xmlrpc_value *subjectv = xmlrpc_string_new(&env, SUBJECT);
  die_if_fault_occurred(&env);
  char message[BUFSIZ];
  char line[BUFSIZ];
  FILE *msgf = fopen(STUDYDATADIR MESSAGE_FILE, "r");
  if (msgf == NULL)
  {
    err(EXIT_FAILURE, "fopen");
  }
  while ((tmp = fgets(line, BUFSIZ, msgf)) != NULL)
  {
    char *p = strpbrk(toaddress, "\r\n");
    if (p != NULL)
    {
      *p = '\0';
    }
    strcat(message, tmp);
  }
  fclose(msgf);
  xmlrpc_value *messagev = xmlrpc_string_new(&env, message);
  die_if_fault_occurred(&env);
  xmlrpc_value *encodingTypev = xmlrpc_int_new(&env, 2);
  die_if_fault_occurred(&env);
  xmlrpc_value *TTLv = xmlrpc_int_new(&env, 28 * 4 * 24 * 60 * 60);
  die_if_fault_occurred(&env);
  fprintf(stderr, "initialized\n");
  FILE *toaddrfile = fopen(STUDYDATADIR SENDTO_ADDRESS_FILE, "r");
  if (toaddrfile == NULL)
  {
    err(EXIT_FAILURE, "fopen");
  }

  while ((tmp = fgets(toaddress, 64, toaddrfile)) != NULL)
  {
    /* ファイルから読み込んだ文字列から改行文字を取り除く */
    char *p = strpbrk(toaddress, "\r\n");
    if (p != NULL)
    {
      *p = '\0';
    }
    /* 文字列をxmlrpc文字列オブジェクトに変換する */
    toaddressv = xmlrpc_string_new(&env, toaddress);
    die_if_fault_occurred(&env);

    /* xmlrpcのパラメータを組み立てる */
    paramArray = xmlrpc_array_new(&env);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, toaddressv);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, fromaddressv);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, subjectv);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, messagev);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, encodingTypev);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, TTLv);
    die_if_fault_occurred(&env);

    /* Make the remote procedure call パラメーターとメソッドを指定して呼び出す */
    xmlrpc_client_call2(&env, clientP, serverP, METHOD_NAME, paramArray, &resultP);
    die_if_fault_occurred(&env);

    printf("%s\n", toaddress);

    /* Dispose of our result value. ゴミ掃除 */
    xmlrpc_DECREF(paramArray);
    xmlrpc_DECREF(toaddressv);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);
    xmlrpc_DECREF(resultP);
  }
  fclose(toaddrfile);
  xmlrpc_DECREF(fromaddressv);
  xmlrpc_DECREF(subjectv);
  xmlrpc_DECREF(messagev);
  xmlrpc_DECREF(encodingTypev);
  xmlrpc_DECREF(TTLv);

  /* Clean up our error-handling environment. */
  xmlrpc_env_clean(&env);

  xmlrpc_client_destroy(clientP);

  xmlrpc_client_teardown_global_const();

  return EXIT_SUCCESS;
}
