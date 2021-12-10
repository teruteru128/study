

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
#include <pthread.h>
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
#define PASSWORD "XXX"
#define ADDRBUFSIZE 64
#ifndef STUDYDATADIR
#define STUDYDATADIR ""
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
    char addressfilepath[PATH_MAX];
    snprintf(addressfilepath, PATH_MAX, "%s%s", STUDYDATADIR,
             SENDTO_ADDRESS_FILE);

#ifdef _DEBUG
    printf("%s\n", addressfilepath);
#endif
    fputs("start\n", stdout);

    xmlrpc_env env;
    xmlrpc_client *clientP = NULL;
    xmlrpc_server_info *serverP = NULL;
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);
    die_if_fault_occurred(&env);

    xmlrpc_client_setup_global_const(&env);
    die_if_fault_occurred(&env);

    // create client object...
    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
                         &clientP);
    die_if_fault_occurred(&env);

    // auth config object
    serverP = xmlrpc_server_info_new(&env, SERVER_URL);
    die_if_fault_occurred(&env);

    // auth config
    xmlrpc_server_info_set_user(&env, serverP, USER_NAME, PASSWORD);
    die_if_fault_occurred(&env);

    // auth enable
    xmlrpc_server_info_allow_auth_basic(&env, serverP);
    die_if_fault_occurred(&env);

    // message params
    char toaddress[ADDRBUFSIZE] = "BM-2cXiKJ5Qm63CqbV58P76HECHdmQUmTV4Fb";
    char fromaddress[] = "BM-NB3mUXqpbGXKQHUP95fx7yqWHPkDTQp8";
    char subject[BUFSIZ] = "dGVzdCBtZXNzYWdlIGJ5IGM=";
    char message[BUFSIZ] = "44GG44KT44Gh44O877yB";
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
    fprintf(stderr, "initialized\n");

    // send message
    char *ackdata = bmapi_sendMessage(&env, clientP, serverP, toaddressv, fromaddressv,
                            subjectv, messagev, encodingTypev, TTLv);
    free(ackdata);

    printf("%s\n", toaddress);

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

    return EXIT_SUCCESS;
}
