

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

#define USER_NAME "teruteru128"
#define PASSWORD "testpassword"
#define ADDRBUFSIZE 64
#ifndef STUDYDATADIR
#define STUDYDATADIR PROJECT_SOURCE_DIR "/data"
#endif

int countDownToStartupTime(time_t targetTime)
{
    struct timespec spec;
    spec.tv_sec = 0;
    spec.tv_nsec = 100000000;
    time_t now;
    long secs = 0;
    long days;
    long hours;
    long minutes;
    long seconds;
    while (1)
    {
        nanosleep(&spec, NULL);
        now = time(NULL);
        secs = (long)difftime(targetTime, now);
        if (secs < 0)
        {
            // 現在時刻が起動時刻を超えたらbreak
            break;
        }
        days = secs / (60 * 60 * 24);
        hours = (secs % (60 * 60 * 24)) / (60 * 60);
        minutes = (secs % (60 * 60)) / 60;
        seconds = secs % 60;
        fprintf(stdout, "起動まであと%03ldd%02ldh%02ldm%02lds\r", days, hours, minutes, seconds);
        fflush(stdout);
    }
    fputs("\n", stdout);
    fflush(stdout);
    return EXIT_SUCCESS;
}

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
 * TODO: コマンドライン引数でtest.txtとaddressbook.txtを切り替えられるようにする
 * */
int main(void)
{
    char msgfilepath[PATH_MAX];
    char addressfilepath[PATH_MAX];
    snprintf(msgfilepath, PATH_MAX, "%s%s", STUDYDATADIR, MESSAGE_FILE);
    snprintf(addressfilepath, PATH_MAX, "%s%s", STUDYDATADIR, SENDTO_ADDRESS_FILE);

#ifdef _DEBUG
    printf("%s\n", msgfilepath);
    printf("%s\n", addressfilepath);
#endif

    fputs("起動します\n", stdout);

    // error environment variable
    xmlrpc_env *env = malloc(sizeof(xmlrpc_env));
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(env);
    die_if_fault_occurred(env);
    xmlrpc_init(env);
    die_if_fault_occurred(env);
    xmlrpc_client_setup_global_const(env);
    die_if_fault_occurred(env);

    // create client object...
    xmlrpc_client *clientP = NULL;
    xmlrpc_client_create(env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
                         &clientP);
    die_if_fault_occurred(env);

    // auth config object
    xmlrpc_server_info *serverP = xmlrpc_server_info_new(env, SERVER_URL);
    die_if_fault_occurred(env);

    // auth config
    xmlrpc_server_info_set_user(env, serverP, USER_NAME, PASSWORD);
    die_if_fault_occurred(env);

    // auth enable
    xmlrpc_server_info_allow_auth_basic(env, serverP);
    die_if_fault_occurred(env);

    // message params
    char toaddress[ADDRBUFSIZE] = "";
    char *tmp = NULL;
    xmlrpc_value *fromaddressv = xmlrpc_string_new(env, "BM-2cWy7cvHoq3f1rYMerRJp8PT653jjSuEdY");
    die_if_fault_occurred(env);
    xmlrpc_value *subjectv = xmlrpc_string_new(env, SUBJECT);
    die_if_fault_occurred(env);
    char message[BUFSIZ] = "TWVycnkgQ2hyaXN0bWFzISE=";
    xmlrpc_value *messagev = xmlrpc_string_new(env, message);
    die_if_fault_occurred(env);
    xmlrpc_value *encodingTypev = xmlrpc_int_new(env, 2);
    die_if_fault_occurred(env);
    xmlrpc_value *TTLv = xmlrpc_int_new(env, 28 * 4 * 24 * 60 * 60);
    die_if_fault_occurred(env);
    fprintf(stderr, "initialized\n");
    FILE *toaddrfile = fopen(addressfilepath, "r");
    if (toaddrfile == NULL)
    {
        err(EXIT_FAILURE, "fopen");
    }

    xmlrpc_value *toaddressv = NULL;
    char *ackdata = NULL;
    while ((tmp = fgets(toaddress, ADDRBUFSIZE, toaddrfile)) != NULL)
    {
        /* ファイルから読み込んだ文字列から改行文字を取り除く */
        char *p = strpbrk(toaddress, "\r\n");
        if (p != NULL)
        {
            *p = '\0';
        }
        /* 文字列をxmlrpc文字列オブジェクトに変換する */
        toaddressv = xmlrpc_string_new(env, toaddress);
        die_if_fault_occurred(env);

        ackdata = bmapi_sendMessage(env, clientP, serverP, toaddressv, fromaddressv, subjectv, messagev, encodingTypev, TTLv);
        free(ackdata);

        printf("%s\n", toaddress);

        /* Dispose of our result value. ゴミ掃除 */
        xmlrpc_DECREF(toaddressv);
    }
    fclose(toaddrfile);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);
    free(env);

    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();

    return EXIT_SUCCESS;
}
