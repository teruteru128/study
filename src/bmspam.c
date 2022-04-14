
// USER_NAME=teruteru128 BM_PASSWORD=analbeads
// SERVER_URL=http://192.168.12.5:8442/ ./build/src/bmspam
// ./config/address1.txt USER_NAME=teruteru128 BM_PASSWORD=analbeads
// SERVER_URL=http://192.168.12.5:8442/ ./build/src/bmspam imaginaryaddress.txt
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
#include <locale.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#define ADDRBUFSIZE 64
#ifndef STUDYDATADIR
#define STUDYDATADIR PROJECT_SOURCE_DIR "/data"
#endif

static volatile sig_atomic_t running = 1;

static void handler(int sig, siginfo_t *info, void *ctx)
{
    (void)sig;
    running = 0;
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
 * TODO:
 * コマンドライン引数でtest.txtとaddressbook.txtを切り替えられるようにする
 * */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fputs("アドレスファイルを指定してください\n", stderr);
        return 1;
    }
    const char *addressfilepath = argv[1];
    setlocale(LC_ALL, "");
    size_t addressoffset = 0;
    if (argc >= 3)
    {
        addressoffset = strtoul(argv[2], NULL, 10);
    }

#ifdef _DEBUG
    printf("%s\n", msgfilepath);
    printf("%s\n", addressfilepath);
#endif

    fputs("起動します\n", stdout);

    char *username = getenv("USER_NAME");
    char *password = getenv("BM_PASSWORD");
    char *serverUrl = getenv("SERVER_URL");

    if (username == NULL || strlen(username) == 0)
    {
        fputs("環境変数 USER_NAME にユーザー名を設定してください。\n", stderr);
        return 1;
    }
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
    if (serverUrl == NULL || strlen(serverUrl) == 0)
    {
        fputs(
            "環境変数 SERVER_URL にサーバーアドレスを設定してください。(HTTP "
            "URL, example: http://192.168.12.5:8442/)\n",
            stderr);
        return 1;
    }

    // error environment variable
    xmlrpc_env environment;
    xmlrpc_env *env = &environment;
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
    xmlrpc_server_info *serverP = xmlrpc_server_info_new(env, serverUrl);
    die_if_fault_occurred(env);

    // auth config
    xmlrpc_server_info_set_user(env, serverP, username, password);
    die_if_fault_occurred(env);

    // auth enable
    xmlrpc_server_info_allow_auth_basic(env, serverP);
    die_if_fault_occurred(env);

    // message params
    char fromaddress[ADDRBUFSIZE] = ADDRESS_bitmessage;
    char *tmp = NULL;
    char *subject = "";
    const char message[BUFSIZ] = "";
    int encodingType = 2;
    // 3600から2419200まで
    // 物量作戦を採用するならメッセージ本体を小さく、TTLを短く
    // 2419200 = 60*60*24*28
    int ttl = 3600;
    struct sigaction action = { 0 };
    action.sa_sigaction = handler;
    if (sigaction(SIGINT, &action, NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        xmlrpc_client_destroy(clientP);
        xmlrpc_client_teardown_global_const();
        /* Clean up our error-handling environment. */
        xmlrpc_env_clean(env);
        return EXIT_FAILURE;
    }

    xmlrpc_value *fromaddressv = xmlrpc_string_new(env, fromaddress);
    die_if_fault_occurred(env);
    xmlrpc_value *subjectv = xmlrpc_string_new(env, subject);
    die_if_fault_occurred(env);
    xmlrpc_value *messagev = xmlrpc_string_new(env, message);
    die_if_fault_occurred(env);
    xmlrpc_value *encodingTypev = xmlrpc_int_new(env, encodingType);
    die_if_fault_occurred(env);
    xmlrpc_value *TTLv = xmlrpc_int_new(env, ttl);
    die_if_fault_occurred(env);
    fprintf(stderr, "initialized\n");
    FILE *toaddrfile = fopen(addressfilepath, "r");
    if (toaddrfile == NULL)
    {
        err(EXIT_FAILURE, "fopen");
    }

    char toaddress[ADDRBUFSIZE] = "";
    xmlrpc_value *toaddressv = NULL;
    char *ackdata = NULL;
    struct timespec ts = { 0 };
    struct tm machine_tm = { 0 };
    char datetime[BUFSIZ] = "";
    size_t count = 0;
    for (size_t i = 0; i < addressoffset; i++)
    {
        if (fgets(toaddress, ADDRBUFSIZE, toaddrfile) == NULL)
        {
            return 1;
        }
        count++;
    }
    // toaddressってセミコロンつなぎにできないのか？
    while ((tmp = fgets(toaddress, ADDRBUFSIZE, toaddrfile)) != NULL
           && running)
    {
        /* remove crlf */
        char *crlf = strpbrk(toaddress, "\r\n");
        if (crlf != NULL)
        {
            *crlf = '\0';
        }
        /* 文字列をxmlrpc文字列オブジェクトに変換する */
        toaddressv = xmlrpc_string_new(env, toaddress);
        die_if_fault_occurred(env);

        ackdata = bmapi_sendMessage(env, clientP, serverP, toaddressv,
                                    fromaddressv, subjectv, messagev,
                                    encodingTypev, TTLv);
        free(ackdata);

        clock_gettime(CLOCK_REALTIME, &ts);
        localtime_r(&ts.tv_sec, &machine_tm);
        strftime(datetime, BUFSIZ, "%EC%Ey%B%d日 %X %EX", &machine_tm);
        printf("(%zu)%s: %s.%09ld\n", count, toaddress, datetime, ts.tv_nsec);

        /* Dispose of our result value. ゴミ掃除 */
        xmlrpc_DECREF(toaddressv);
        count++;
    }
    if (ferror(toaddrfile))
    {
        perror("ferror(toaddrfile)");
    }
    if (feof(toaddrfile))
    {
    }
    else
    {
        if (running != 1)
        {
            fprintf(stderr, "%zu件\n", count);
        }
    }
    fclose(toaddrfile);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);

    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);

    return EXIT_SUCCESS;
}
