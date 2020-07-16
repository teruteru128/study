

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>
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
  char buf[BUFSIZ];
  char *sendmsg = "";
  char chan_name[64];
  char *msg = NULL;
  bmapi_init();
  if (argc < 2)
  {
    return EXIT_FAILURE;
  }
  FILE *infile = NULL;
  infile = fopen(argv[1], "r");
  if (infile == NULL)
  {
    perror("fopen");
    return EXIT_FAILURE;
  }

  bm_client *c = bm_client_new();

  char *tmp = NULL;
  char *message = "";
  do
  {
    tmp = fgets(chan_name, 64, infile);
    msg = bmapi_simpleSendMessage(chan_name, GENELRAL, "HAPPY NEW YEAR!", message);
    printf("%s\n", msg);
  } while (tmp != NULL);

  xmlrpc_client_teardown_global_const();

  return EXIT_SUCCESS;
}
