

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
  struct tm tm;
  localtime_r(&currentTime.tv_sec, &tm);
  tm.tm_sec = 0;
  tm.tm_min = 0;
  tm.tm_hour = 4;
  tm.tm_mday = 25;
  tm.tm_mon = 11;
  struct timespec christmasTime;
  christmasTime.tv_sec = mktime(&tm);
  christmasTime.tv_nsec = 0;
  double diffsec = difftime(christmasTime.tv_sec, currentTime.tv_sec);
  long diffnsec = christmasTime.tv_nsec - currentTime.tv_nsec;
  if (diffnsec < 0)
  {
    diffnsec += 1000000000;
    diffsec--;
  }
  if (diffsec < 0 || (diffsec == 0 && diffnsec < 0))
  {
    // 今年のクリスマスは終了済み
    printf("日本は終了しました＼(^o^)／\n");
    return 0;
  }

  printf("%.0lf.%09ld\n", diffsec, diffnsec);

  {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_lock(&mutex);
    // 実行時間まで待つ
    int sig = pthread_cond_timedwait(&cond, &mutex, &christmasTime);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    if (sig != ETIMEDOUT)
    {
      return EXIT_FAILURE;
    }
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
  xmlrpc_value *messagev = xmlrpc_string_new(&env, MESSAGE);
  die_if_fault_occurred(&env);
  xmlrpc_value *encodingTypev = xmlrpc_int_new(&env, 2);
  die_if_fault_occurred(&env);
  xmlrpc_value *TTLv = xmlrpc_int_new(&env, 28 * 4 * 24 * 60 * 60);
  die_if_fault_occurred(&env);
  fprintf(stderr, "initialized\n");
  char *p = NULL;
  FILE *toaddrfile = fopen(SENDTO_ADDRESS_FILE, "r");
  if (toaddrfile == NULL)
  {
    err(EXIT_FAILURE, "fopen");
  }

  while ((tmp = fgets(toaddress, 64, toaddrfile)) != NULL)
  {
    /* ファイルから読み込んだ文字列から改行文字を取り除く */
    p = strpbrk(toaddress, "\r\n");
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
