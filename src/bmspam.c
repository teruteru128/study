

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
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442
#define NAME "TR BM TEST CLIENT"
#define SERVER_URL "http://127.0.0.1:8442/"
#define GENELRAL "BM-2cW67GEKkHGonXKZLCzouLLxnLym3azS8r"

static void die_if_fault_occurred(xmlrpc_env *env)
{
  /* Check our error-handling environment for an XML-RPC fault. */
  if (env->fault_occurred)
  {
    fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
            env->fault_string, env->fault_code);
    exit(1);
  }
}

/**
 * int main(int argc, char* argv[]){
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
 * */
int main(int const argc, const char **const argv)
{
  xmlrpc_env env;
  xmlrpc_client *clientP;
  xmlrpc_server_info *serverP;
  xmlrpc_value *resultP;
  const char *msg;
  char *const methodName = "helloWorld";

  /* Initialize our error-handling environment. */
  xmlrpc_env_init(&env);

  xmlrpc_client_setup_global_const(&env);

  xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
                       &clientP);
  die_if_fault_occurred(&env);
  serverP = xmlrpc_server_info_new(&env, SERVER_URL);
  xmlrpc_server_info_set_user(&env, serverP, "teruteru128", "testpassword");
  xmlrpc_server_info_allow_auth_basic(&env, serverP);

  xmlrpc_value *paramArray = xmlrpc_array_new(&env);
  xmlrpc_value *f = xmlrpc_string_new(&env, "hello");
  xmlrpc_value *s = xmlrpc_string_new(&env, "world");
  xmlrpc_array_append_item(&env, paramArray, f);
  xmlrpc_array_append_item(&env, paramArray, s);

  /* Make the remote procedure call */
  xmlrpc_client_call2(&env, clientP, serverP, methodName, paramArray, &resultP);
  die_if_fault_occurred(&env);

  /* Get our sum and print it out. */
  xmlrpc_read_string(&env, resultP, &msg);
  die_if_fault_occurred(&env);
  printf("The sum  is %s\n", msg);
  free((void *)msg);

  /* Dispose of our result value. */
  xmlrpc_DECREF(paramArray);
  xmlrpc_DECREF(resultP);

  /* Clean up our error-handling environment. */
  xmlrpc_env_clean(&env);

  xmlrpc_client_destroy(clientP);

  xmlrpc_client_teardown_global_const();

  return EXIT_SUCCESS;
}
