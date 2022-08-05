
#include <bm.h>
#include <bmapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "TR BM TEST CLIENT"

int function_bcast(const int argc, const char **argv)
{
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
        fputs("環境変数 BM_PASSWORD にパスワードを設定してください。(長さ0のパスワードは一応許容しています)\n",
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
            "URL, example: http://192.168.1.1:8442/)\n",
            stderr);
        return 1;
    }

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

    char *ackdata = NULL;
    bmapi_sendBroadcast(env, clientP, serverP, "BM-2cWytQ6fyG6c69ofik1kmK5aB3nCJCDb47", "U2FtcGxlIGJyb2FkY2FzdA==", "SGVsbG8h", 2, 4 * 86400,
                        &ackdata);
    free(ackdata);

    xmlrpc_server_info_free(serverP);
    xmlrpc_client_destroy(clientP);

    xmlrpc_client_teardown_global_const();

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);
    return 0;
}

/**
 * サブコマンド方式でできねえかな
 * addrgen
 *   新規アドレス生成
 *     乱数ソース
 *     既存鍵ソース
 *     パスフレーズ
 *       バージョン3
 *       バージョン4
 * sendmsg [fromaddress] [toaddress] [subject] [body]
 *   メッセージ送信
 *     PoWサーバー指定機能
 * bcast [fromaddress] [subject] [body] <moreoptions>
 *  配信
 */
int main(const int argc, const char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "引数が少なすぎます");
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "bcast") == 0)
    {
        function_bcast(argc, argv);
    }
    return 0;
}
