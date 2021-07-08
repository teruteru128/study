

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
#define STUDYDATADIR ""
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
    /* 次の実行日時を取得する */
    /* 現在時刻を取得する */
    const time_t currentTime = time(NULL);
    tzset();
    // 今年のクリスマスのUNIXタイムスタンプを取得する
    struct tm tm;
    localtime_r(&currentTime, &tm);
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 4;
    tm.tm_mday = 25;
    tm.tm_mon = 11;
    const time_t christmasTime = mktime(&tm);
    // 実行判定。起動時点で現在時刻が今年のクリスマスよりも未来だった場合実行しない
    double diffsec = difftime(christmasTime, currentTime);
    if (diffsec < 0)
    {
        // 今年のクリスマスは終了済み
        printf("日本は終了しました＼(^o^)／\n");
        printf("終了します\n");
        return EXIT_SUCCESS;
    }
    char msgfilepath[PATH_MAX];
    char addressfilepath[PATH_MAX];
    snprintf(msgfilepath, PATH_MAX, "%s%s", STUDYDATADIR, MESSAGE_FILE);
    snprintf(addressfilepath, PATH_MAX, "%s%s", STUDYDATADIR, SENDTO_ADDRESS_FILE);

#ifdef _DEBUG
    printf("%s\n", msgfilepath);
    printf("%s\n", addressfilepath);
#endif

    // 起動時間までカウントダウンする
    int r = countDownToStartupTime(christmasTime);
    if (r != EXIT_SUCCESS)
    {
        return r;
    }

    fputs("起動します\n", stdout);

    xmlrpc_env env;
    xmlrpc_client *clientP;
    xmlrpc_server_info *serverP;
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);
    die_if_fault_occurred(&env);
    xmlrpc_client_setup_global_const(&env);
    die_if_fault_occurred(&env);
    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0, &clientP);
    die_if_fault_occurred(&env);
    serverP = xmlrpc_server_info_new(&env, SERVER_URL);
    die_if_fault_occurred(&env);
    xmlrpc_server_info_set_user(&env, serverP, USER_NAME, PASSWORD);
    die_if_fault_occurred(&env);
    xmlrpc_server_info_allow_auth_basic(&env, serverP);
    die_if_fault_occurred(&env);

    char toaddress[ADDRBUFSIZE];
    char *tmp = NULL;
    xmlrpc_value *toaddressv = NULL;
    xmlrpc_value *fromaddressv = xmlrpc_string_new(&env, GENELRAL);
    die_if_fault_occurred(&env);
    xmlrpc_value *subjectv = xmlrpc_string_new(&env, SUBJECT);
    die_if_fault_occurred(&env);
    char message[BUFSIZ];
    char line[BUFSIZ];
    FILE *msgf = fopen(msgfilepath, "r");
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
        strncat(message, tmp, BUFSIZ - 1);
    }
    fclose(msgf);
    xmlrpc_value *messagev = xmlrpc_string_new(&env, message);
    die_if_fault_occurred(&env);
    xmlrpc_value *encodingTypev = xmlrpc_int_new(&env, 2);
    die_if_fault_occurred(&env);
    xmlrpc_value *TTLv = xmlrpc_int_new(&env, 28 * 4 * 24 * 60 * 60);
    die_if_fault_occurred(&env);
    fprintf(stderr, "initialized\n");
    FILE *toaddrfile = fopen(addressfilepath, "r");
    if (toaddrfile == NULL)
    {
        err(EXIT_FAILURE, "fopen");
    }

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
        toaddressv = xmlrpc_string_new(&env, toaddress);
        die_if_fault_occurred(&env);

        ackdata = bmapi_sendMessage(&env, clientP, serverP, toaddressv, fromaddressv, subjectv, messagev, encodingTypev, TTLv);
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
    xmlrpc_env_clean(&env);

    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();

    return EXIT_SUCCESS;
}
