

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "bmspam.h"
#include <base64.h>
#include <bitmessage.h>
#include <bm.h>
#include <bmapi.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <omp.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#define USER_NAME "teruteru128"
#define ADDRBUFSIZE 64
#ifndef STUDYDATADIR
#define STUDYDATADIR PROJECT_SOURCE_DIR "/data"
#endif

/**
 * @brief HAHAHA!  BM-NB3mUXqpbGXKQHUP95fx7yqWHPkDTQp8
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
 * TODO:
 * コマンドライン引数でtest.txtとaddressbook.txtを切り替えられるようにする
 * */
int main(void)
{
    fputs("start\n", stdout);

    char *password = getenv("BM_PASSWORD");

    if (password == NULL)
    {
        fputs("環境変数 BM_PASSWORD にパスワードを設定してください。\n",
              stderr);
        return 1;
    }
    if (strlen(password) == 0)
    {
        fputs("環境変数 BM_PASSWORD の長さが0だが、それでええんか？\n",
              stderr);
    }

    // error environment variable
    xmlrpc_env env;
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);
    die_if_fault_occurred(&env);
    xmlrpc_init(&env);
    die_if_fault_occurred(&env);
    xmlrpc_client_setup_global_const(&env);
    die_if_fault_occurred(&env);

    // create client object...
    xmlrpc_client *clientP = NULL;
    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
                         &clientP);
    die_if_fault_occurred(&env);

    // connection info & auth config object
    xmlrpc_server_info *serverP = xmlrpc_server_info_new(&env, SERVER_URL);
    die_if_fault_occurred(&env);

    // auth config
    xmlrpc_server_info_set_user(&env, serverP, USER_NAME, password);
    die_if_fault_occurred(&env);

    // auth enable
    xmlrpc_server_info_allow_auth_basic(&env, serverP);
    die_if_fault_occurred(&env);

    // message params
    char toaddress[ADDRBUFSIZE] = "BM-2cV4tWm832NcVUJqG65bmFd64hVD7nUizL";
    char fromaddress[] = "BM-NBJxKhQmidR2TBtD3H74yZhDHpzZ7TXM";
    const char subject[BUFSIZ] = "44GG44KT44Gh44GX44Gm77yf";
    const char message[BUFSIZ] = "5auM44Gn44GZ4oCm4oCm";
    int ttl = 2419200;

    /*
     文字列をxmlrpc文字列オブジェクトに変換する
     Convert a string to an xmlrpc string object
     */
    xmlrpc_value *fromaddressv = xmlrpc_string_new(&env, fromaddress);
    die_if_fault_occurred(&env);
    xmlrpc_value *toaddressv = xmlrpc_string_new(&env, toaddress);
    die_if_fault_occurred(&env);
    xmlrpc_value *subjectv = xmlrpc_string_new(&env, subject);
    die_if_fault_occurred(&env);
    xmlrpc_value *messagev = xmlrpc_string_new(&env, message);
    die_if_fault_occurred(&env);
    xmlrpc_value *encodingTypev = xmlrpc_int_new(&env, 2);
    die_if_fault_occurred(&env);
    xmlrpc_value *TTLv = xmlrpc_int_new(&env, ttl);
    die_if_fault_occurred(&env);
    fputs("initialized\n", stderr);

    // send message
    char *ackdata = NULL;
    bmapi_sendMessage(&env, clientP, serverP, toaddressv, fromaddressv,
                      subjectv, messagev, encodingTypev, TTLv, &ackdata);
    free(ackdata);

    /* Dispose of our result value. ゴミ掃除 */
    xmlrpc_DECREF(toaddressv);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);

    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();
    xmlrpc_term();

    return EXIT_SUCCESS;
}
