

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
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    const char *addressfilepath = argv[1];
    setlocale(LC_ALL, "");

#ifdef _DEBUG
    printf("%s\n", msgfilepath);
    printf("%s\n", addressfilepath);
#endif

    fputs("起動します\n", stdout);

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
    xmlrpc_server_info *serverP = xmlrpc_server_info_new(env, SERVER_URL);
    die_if_fault_occurred(env);

    // auth config
    xmlrpc_server_info_set_user(env, serverP, USER_NAME, password);
    die_if_fault_occurred(env);

    // auth enable
    xmlrpc_server_info_allow_auth_basic(env, serverP);
    die_if_fault_occurred(env);

    // message params
    char fromaddress[ADDRBUFSIZE] = ADDRESS_bitmessage;
    char *tmp = NULL;
    char *subject = NULL;
    BIO *bio_subject, *b64;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio_subject = BIO_new(BIO_s_mem());
    BIO_get_mem_ptr(bio_subject, &subject);
    BIO_set_close(bio_subject, BIO_NOCLOSE);
    BIO_push(b64, bio_subject);
    //BIO_write(b64, message, strlen(message));
    BIO_flush(b64);
    BIO_free_all(b64);
    char message[BUFSIZ] = "SGVsbG8gV29ybGQh";
    int encodingType = 2;
    // 2419200 = 60*60*24*28
    int ttl = 2419200;

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
    time_t t = 0;
    struct tm machine_tm;
    char strtime[BUFSIZ] = "";
    size_t count = 0;
    while ((tmp = fgets(toaddress, ADDRBUFSIZE, toaddrfile)) != NULL)
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

        t = time(NULL);
        localtime_r(&t, &machine_tm);
        strftime(strtime, BUFSIZ, "%EC%Ey%B%d日 %X %EX", &machine_tm);
        printf("(%zu)%s: %s\n", count, toaddress, strtime);

        /* Dispose of our result value. ゴミ掃除 */
        xmlrpc_DECREF(toaddressv);
        sleep(1);
    }
    if (ferror(toaddrfile))
    {
        perror("ferror(toaddrfile)");
    }
    fclose(toaddrfile);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);

    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();

    return EXIT_SUCCESS;
}
