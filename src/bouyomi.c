/*
    今回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
    前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
*/

#include "bouyomi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()
#include <iconv.h>
#include <limits.h>
#include <locale.h>
#include <wchar.h>
#include <netdb.h>
#include <errno.h>
#include "charset-convert.h"

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)
#define DEFAULT_PORT 50001
#define DEFAULT_PORT_STR "50001"
#define DEFAULT_SERV_ADDRESS "localhost"
#define DEFAULT_SERV_ADDRESS4 "127.0.0.1"
#define DEFAULT_SERV_ADDRESS6 "::1"
#define ONION_SERV_ADDRESS "2ayu6gqru3xzfzbvud64ezocamykp56kunmkzveqmuxvout2yubeeuad.onion"

/**
 * アドレスとポート番号を表示する。
 * <I> adrinf: アドレス情報
 */
static void print_addrinfo(struct addrinfo *adrinf) {
  char hbuf[NI_MAXHOST];  /* 返されるアドレスを格納する */
  char sbuf[NI_MAXSERV];  /* 返されるポート番号を格納する */
  int rc;

  /* アドレス情報に対応するアドレスとポート番号を得る */
  rc = getnameinfo(adrinf->ai_addr, adrinf->ai_addrlen,
            hbuf, sizeof(hbuf),
            sbuf, sizeof(sbuf),
            NI_NUMERICHOST | NI_NUMERICSERV);
  if (rc != 0) {
    fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(rc));
    return;
  }

  fprintf(stderr, "[%s]:%s\n", hbuf, sbuf);
}

typedef struct bouyomi_conf_t{
    short command;
    short speed;
    short tone;
    short volume;
    short voice;
    char encode;
    size_t length;
    char* msg;
} bouyomi_conf;
typedef struct config_line_t{
  char* key;
  char* value;
  struct config_line_t* next;
} config_line_t;

typedef enum charset_t {
  UTF_8 = 0,
  UNICODE = 1,
  SHIFT_JIS = 2
} charset;

int send_to_server(char* hostname, char*servicename, char* data, size_t len){
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  int rc = getaddrinfo(hostname, servicename, &hints, &res);
  if(rc!=0){
    perror("getaddrinfo");
    return EXIT_FAILURE;
  }

  struct addrinfo *adrinf = NULL;
  int sock = 0;
  for(adrinf = res; adrinf != NULL; adrinf = adrinf->ai_next){
    sock = socket(adrinf->ai_family, adrinf->ai_socktype, adrinf->ai_protocol);
    if(sock<0){
      perror("socket()");
      exit(EXIT_FAILURE);
    }
    rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
    if(rc<0){
      close(sock);
      continue;
    }
    print_addrinfo(adrinf);
    break;
  }
  freeaddrinfo(res);
  if(rc<0){
    perror("connect()");
    close(sock);
    exit(EXIT_FAILURE);
  }

  ssize_t w = 0;
  // 送信
  w = write(sock, data, len);
  fprintf(stderr, "送信しました！\n");
  // ソケットを閉じる
  rc = close(sock);
  if(rc != 0){
    perror("close");
    return EXIT_FAILURE;
  }

}

/*
  1. encode
  2. connect
  3. send
  4. cleanup

  option
  host & port
  charset
*/
int main(int argc, char* argv[]){
  int rc = 0;
  int ignore_errors = 0;

  if(setlocale(LC_ALL, "ja_JP.UTF-8") == NULL){
    perror("setlocale");
    return EXIT_FAILURE;
  }
  /*
https://github.com/torproject/tor/blob/03867b3dd71f765e5adb620095692cb41798c273/src/app/config/config.c#L2537
parsed_cmdline_t* config_parse_commandline(int argc, char **argv, int ignore_errors)
*/
  /*
     {
     char *s, *arg;
     int i = 1;
     while(i < argc){
     int command = 0; // CONFIG_LINE_NORMAL
     takes_argument_t want_arg = ARGUMENT_NECESSARY;
     int is_cmdline = 0;
     int j;
     int is_a_command = 0;

     for(j = 0; option_list[j].name != NULL; j++){
     if(!strcmp(argv[i], option_list[j].name)){
     is_cmdline = 1;
     want_arg = option_list[i].takes_argument;
     break;
     }
     }

     s = argv[i];

     const int is_last = (i == argc-1);
     if(want_arg == ARGUMENT_NECESSARY && is_last){
     if(ignore_errors){}else{}
     } else if(want_arg == ARGUMENT_OPTIONAL && is_last){
     }else{
     arg = (want_arg != ARGUMENT_NONE) ? strdup(argv[i + 1]) : strdup("");
     }

     config_line_t* param = malloc(sizeof(config_line_t));
     memset(param, 0, sizeof(config_line_t));
     param->key = is_cmdline ? strdup(argv[i]):strdup(""); // abberv
     param->value = arg;
     param->command = command;
     param->next = NULL;

     if(is_a_command){
     result->command_arg = param->value;
     }

     if(is_cmdline){
   *new_cmdline = param;
   new_cmdline = &((*new_cmdline)->next);
   }else{
   *new = param;
   new = &((*new)->next);
   }

   i += want_arg ? 2: 1;
   }
   }*/
  char *in = NULL;
  if(argc >= 2){
    in = argv[1];
  } else {
    in = "テストでーす";
  }
  charset charset = UTF_8;
  char *out = NULL;

  if(charset == UTF_8){
    //  エンコード用変数にそのまま代入
    // freeするために複製を入れる
    out = strdup(in);
  } else if(charset == UNICODE){
    //  文字コードを変換してから代入
    encode_utf8_2_unicode(&out, in);
  } else if(charset == SHIFT_JIS){
    //  文字コードを変換してから代入
    encode_utf8_2_sjis(&out, in);
  }

  // TODO encode関数に分離
  // 棒読みちゃん向けにエンコード
  char header[15]; //で書き直す
  short command = 1;
  short speed = -1;
  short tone = -1;
  short volume = -1;
  short voice = 0;
  char encode;
  if(charset == UTF_8){
    encode = 0;
  } else if(charset == UNICODE){
    encode = 1;
  } else if(charset == SHIFT_JIS){
    encode = 2;
  }
  size_t length = strlen(out);
  fprintf(stderr, "length : %ld\n", length);

  // なぜhtonsなしで読み上げできるのか謎
  // 棒読みちゃんはリトルエンディアン指定だそうです
  *((short*)&header[0]) = command;
  *((short*)&header[2]) = speed;
  *((short*)&header[4]) = tone;
  *((short*)&header[6]) = volume;
  *((short*)&header[8]) = voice;
  *((char*)&header[10]) = encode;
  *((int*)&header[11]) = length;

  size_t send_len = 15 + length;
  char* send_buf = malloc(send_len);
  memcpy(send_buf, header, 15);
  memcpy(send_buf + 15, out, length);
  free(out);

  char *servAddr = ONION_SERV_ADDRESS;
  char *servPortStr = DEFAULT_PORT_STR;
  int use_onion = 1;
  if(use_onion == 1){
    servAddr = ONION_SERV_ADDRESS;
  }else{
    servAddr = DEFAULT_SERV_ADDRESS;
  }
  rc = send_to_server(servAddr, servPortStr, send_buf, send_len);
  free(send_buf);
  return EXIT_SUCCESS;
}
