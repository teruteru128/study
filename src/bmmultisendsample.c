
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

int main(int argc, char const *argv[])
{
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
    char *subject = "テスト";
    const char message[BUFSIZ] = "テトリス";
    int encodingType = 2;
    // 3600から2419200まで
    // 物量作戦を採用するならメッセージ本体を小さく、TTLを短く
    // 2419200 = 60*60*24*28
    int ttl = 86400 * 4;

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

    char toaddress[128]
        = "BM-FHmDQSDbdMPfupgfJRmwcrYxyXWW;BM-FHhkCqEMdaDSfLp8NjpSXSA9H7Td";
    xmlrpc_value *toaddressv = xmlrpc_string_new(env, toaddress);
    char *ackdata
        = bmapi_sendMessage(env, clientP, serverP, toaddressv, fromaddressv,
                            subjectv, messagev, encodingTypev, TTLv);
    free(ackdata);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);

    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);
    return 0;
}
