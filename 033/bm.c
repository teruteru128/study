
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>
#include <string.h>
#include "bitmessage.h"
#include "base64.h"
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
typedef struct clientinfo_t {} clientinfo;
typedef struct serverinfo_t {} serverinfo;
typedef struct connectioninfo_t {
  clientinfo client;
  serverinfo server;
} connectioninfo;
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
    const char * msg = NULL;
    char *  generaladdress = "BM-2cW67GEKkHGonXKZLCzouLLxnLym3azS8r";
    char *  fromaddress = "BM-NBJxKhQmidR2TBtD3H74yZhDHpzZ7TXM";
    char *  toaddress = "BM-NBJxKhQmidR2TBtD3H74yZhDHpzZ7TXM";
    fromaddress = generaladdress;
    api_init();

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
    /*
    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, clientName, clientVersion, NULL, 0, &cp);
    die_if_fault_occurred(&env);

    sinfo = xmlrpc_server_info_new(&env, url);
    die_if_fault_occurred(&env);

    xmlrpc_server_info_set_user(&env, sinfo, "teruteru128", "testpassword");
    die_if_fault_occurred(&env);

    xmlrpc_server_info_allow_auth_basic(&env, sinfo);
    */
    FILE* fp = fopen("./bitmessage-addresses.txt", "r");
    if(fp == NULL){
      perror("fopen");
      return EXIT_FAILURE;
    }
    char address[64];
    int i=0;
    while((fgets(address, 64, fp)) != NULL){
      if(strlen(address) == 0){
        continue;
      }
      for(i = 0; address[i] != 0;i++){
        if(address[i] == '\n' || address[i] == '\r'){
          address[i] = '\0';
        }
      }
      msg = api_simpleSendMessage(address, generaladdress, "Sex!", "...and the City.\n\nDid you expect?\n\nYou are an idiot!\n\nHave A Nice New Year!");
      printf("message : %s\n", msg);
      free((void *)msg);
      msg = NULL;
    }
    /*
    //1687-found-chan-names.txt
    FILE* fp = fopen("./1687-found-chan-names.txt", "r");
    if(fp == NULL){
      perror("fopen");
      return EXIT_FAILURE;
    }
    char address[1025];
    int i=0;
    while((fgets(address, 1025, fp)) != NULL){
      for(i = 0; address[i] != 0;i++){
        if(address[i] == '\r'||address[i] == '\n'){
          address[i] = '\0';
        }
      }
      msg = api_getDeterministicAddress(address, 4, 1);
      printf("%s\n", msg);
      free((void *)msg);
      msg = NULL;
    }
    */
    /*
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
    */
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
    /*
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
    */

    //xmlrpc_client_destroy(cp);
    //xmlrpc_DECREF(paramArray);
    //xmlrpc_server_info_free(sinfo);
    /* Dispose of our result value. */
    //xmlrpc_DECREF(resultP);

    /* Clean up our error-handling environment. */
    //xmlrpc_env_clean(&env);
    /* Shutdown our XML-RPC client library. */

    //xmlrpc_client_cleanup();
    xmlrpc_client_teardown_global_const();

    return 0;
}

