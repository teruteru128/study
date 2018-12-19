
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>
#include <string.h>
#include "bitmessage.h"
#include "api.h"
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442
#define NAME "TR BM TEST CLIENT"
#define VERSION "0.0.1-alpha"
#define SERVER_URL "http://127.0.0.1:8442/"

void die_if_fault_occurred (xmlrpc_env *env)
{
    /* Check our error-handling environment for an XML-RPC fault. */
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        exit(1);
    }
}
/**
 * int main(int argc, char* argv[]){
    // グローバル定数初期化
    global_init();
    // 設定ファイルパース
    configfile_parse();
    // コマンドライン引数パース
    arg_parse();
    // メイン処理
    do_main();
    // 後片付け
    global_cleanup();
  }
 * */
int main(int const argc, const char ** const argv)
{

    xmlrpc_env env;
    xmlrpc_value * resultP = NULL;
    xmlrpc_server_info *sinfo = NULL;
    const char * msg = NULL;
    char * const clientName = NAME;
    char * const clientVersion = VERSION;
    char * const url = SERVER_URL;
    char * const methodName = "helloWorld";
    char * const fromaddress = "BM-NBJxKhQmidR2TBtD3H74yZhDHpzZ7TXM";
    char * const toaddress = "BM-2cVogWZyryp9ZDGPQJ6GMpawpofm5oLKYY";
    xmlrpc_client* cp = NULL;
    xmlrpc_value* paramArray = NULL;
    xmlrpc_value* first = NULL;
    xmlrpc_value* second = NULL;
    xmlrpc_value* toaddressV = NULL;
    xmlrpc_value* fromaddressV = NULL;
    xmlrpc_value* subjectV = NULL;
    xmlrpc_value* messageV = NULL;
    xmlrpc_value* ackdata = NULL;

    // api_init();
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Start up our XML-RPC client library. */
    //xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, clientName, clientVersion, NULL, 0);
    xmlrpc_client_setup_global_const(&env);
    //[ handle possible failure of above ]

    typedef struct clientinfo_t {} clientinfo;
    typedef struct serverinfo_t {} serverinfo;
    typedef struct connectioninfo_t {
      clientinfo client;
      serverinfo server;
    } connectioninfo;
    /**
    void
    xmlrpc_client_create(xmlrpc_env *                envP,
                         int                         flags,
                         char *                      appname,
                         char *                      appversion,
                         struct xmlrpc_clientparms * clientparmsP,
                         unsigned int                parmSize,
                         xmlrpc_client **            clientPP);
    */
    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, clientName, clientVersion, NULL, 0, &cp);
    die_if_fault_occurred(&env);

    sinfo = xmlrpc_server_info_new(&env, url);
    die_if_fault_occurred(&env);

    xmlrpc_server_info_set_user(&env, sinfo, "teruteru128", "testpassword");
    die_if_fault_occurred(&env);

    xmlrpc_server_info_allow_auth_basic(&env, sinfo);

    paramArray = xmlrpc_array_new(&env);
    die_if_fault_occurred(&env);

    first = xmlrpc_string_new(&env, "Tell your");
    die_if_fault_occurred(&env);

    second = xmlrpc_string_new(&env, "World!");
    die_if_fault_occurred(&env);

    xmlrpc_array_append_item(&env, paramArray, first);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, second);
    die_if_fault_occurred(&env);
    /*
    toaddressV = xmlrpc_string_new(&env, toaddress);
    die_if_fault_occurred(&env);

    fromaddressV = xmlrpc_string_new(&env, fromaddress);
    die_if_fault_occurred(&env);

    subjectV = xmlrpc_string_new(&env, "dG9vbHRlc3Q=\n");
    die_if_fault_occurred(&env);

    messageV = xmlrpc_string_new(&env, "SGVsbG8gV29ybGQh\n");
    die_if_fault_occurred(&env);

    xmlrpc_array_append_item(&env, paramArray, toaddressV);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, fromaddressV);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, subjectV);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, messageV);
    die_if_fault_occurred(&env);
    */
    /*
    ackdata = xmlrpc_string_new(&env, "2d672b4bb31b3d8cb96e587e05f799f0d4d9c102f588581fbd0fdd082f150c1f");
    die_if_fault_occurred(&env);

    xmlrpc_array_append_item(&env, paramArray, ackdata);
    die_if_fault_occurred(&env);

*/
    xmlrpc_client_call2(&env, cp, sinfo, methodName, paramArray, &resultP);
    die_if_fault_occurred(&env);
    printf("XMLRPC_TYPE_INT : %d\n", XMLRPC_TYPE_INT);
    printf("XMLRPC_TYPE_BOOL %d\n", XMLRPC_TYPE_BOOL);
    printf("XMLRPC_TYPE_DOUBLE : %d\n", XMLRPC_TYPE_DOUBLE);
    printf("XMLRPC_TYPE_DATETIME : %d\n", XMLRPC_TYPE_DATETIME);
    printf("XMLRPC_TYPE_STRING :  %d\n", XMLRPC_TYPE_STRING);
    printf("XMLRPC_TYPE_BASE64 : %d\n", XMLRPC_TYPE_BASE64);
    printf("XMLRPC_TYPE_ARRAY : %d\n", XMLRPC_TYPE_ARRAY);
    printf("XMLRPC_TYPE_STRUCT : %d\n", XMLRPC_TYPE_STRUCT);
    printf("XMLRPC_TYPE_C_PTR : %d\n", XMLRPC_TYPE_C_PTR);
    printf("XMLRPC_TYPE_NIL : %d\n", XMLRPC_TYPE_NIL);
    printf("XMLRPC_TYPE_I8 : %d\n", XMLRPC_TYPE_I8);
    printf("%d\n", xmlrpc_value_type(resultP));

    //xmlrpc_parse_value(&env, resultP, "s", &msg);
    xmlrpc_read_string(&env, resultP, &msg);
    die_if_fault_occurred(&env);

    printf("message : %s\n", msg);

    free((void *)msg);
    msg = NULL;

    xmlrpc_client_destroy(cp);
    xmlrpc_DECREF(paramArray);
    xmlrpc_server_info_free(sinfo);
    /* Dispose of our result value. */
    xmlrpc_DECREF(resultP);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);
    /* Shutdown our XML-RPC client library. */

    //xmlrpc_client_cleanup();
    xmlrpc_client_setup_global_const(&env);

    return 0;
}

