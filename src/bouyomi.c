/*
 * 今回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
 * 前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
 */

#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include "bouyomi.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <limits.h>
#include <locale.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <errno.h>
#include <charset-convert.h>
#include <print_addrinfo.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <iconv.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h> //close()
#endif

/**
 * 
 */
int send_to_server(char *hostname, char *servicename, char *data, size_t len)
{
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  int rc = getaddrinfo(hostname, servicename, &hints, &res);
  if (rc != 0)
  {
    perror("getaddrinfo");
    return EXIT_FAILURE;
  }

  struct addrinfo *adrinf = NULL;
  int sock = 0;
  for (adrinf = res; adrinf != NULL; adrinf = adrinf->ai_next)
  {
    sock = socket(adrinf->ai_family, adrinf->ai_socktype, adrinf->ai_protocol);
    if (sock < 0)
    {
      perror("socket()");
      exit(EXIT_FAILURE);
    }
    rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
    if (rc < 0)
    {
      close(sock);
      continue;
    }
    print_addrinfo(adrinf);
    break;
  }
  freeaddrinfo(res);
  if (rc < 0)
  {
    perror("connect()");
    close(sock);
    exit(EXIT_FAILURE);
  }

  ssize_t w = 0;
  // 送信
  if ((w = write(sock, data, len)) <= 0)
  {
    fprintf(stderr, _("Error! %s\n"), strerror(errno));
  }
  else
  {
    fprintf(stderr, _("Sent! %ld\n"), w);
  }
  // ソケットを閉じる
  rc = close(sock);
  if (rc != 0)
  {
    perror("close");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/*
 * 1. コマンドライン引数解析
 * 2. 読み上げメッセージ文字コード変換
 * 3. プロトコルエンコード
 * 4. サーバーへ接続
 * 5. 後始末
 *
 * 1. encode
 * 2. connect
 * 3. send
 * 4. cleanup
 *
 * option
 * server(host & port)
 * charset
 * proxyは外部で対処
 * 
 * bouyomic *bouyomi_client_new();
 */
int main(int argc, char *argv[])
{
  int rc = 0;
  int ignore_errors = 0;

  setlocale(LC_ALL, "");
  /*
  if (setlocale(LC_ALL, "ja_JP.UTF-8") == NULL)
  {
    perror("setlocale");
    return EXIT_FAILURE;
  }
  */
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  /*
   * https://github.com/torproject/tor/blob/03867b3dd71f765e5adb620095692cb41798c273/src/app/config/config.c#L2537
   * parsed_cmdline_t* config_parse_commandline(int argc, char **argv, int ignore_errors)
   * --host
   * --port
   * --command
   * --speed
   * --tone
   * --volume
   * --voice
   * args *new_args();
   * call_bouyomi(int argc, char **argv);
   *  parseargs
   *  buildrequest
   *  chooseserver
   *  sendtoserver
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
  char *in = malloc(1);
  in[0] = 0;
  char *tmp = NULL;
  size_t strlength = 0;
  size_t len = 0;
  char buf[BUFSIZ];
  if(!isatty(fileno(stdin))){
  while (fgets(buf, BUFSIZ, stdin) != NULL)
  {
    len = strlen(buf);
    if (len == 0)
    {
      continue;
    }
    tmp = realloc(in, strlength + len + 1);
    if (!tmp)
    {
      exit(EXIT_FAILURE);
    }
    in = tmp;
    strcat(in + strlength, buf);
    strlength += len;
  }
  }
  if (strlength == 0)
  {
    len = strlen("やったぜ。");
    tmp = realloc(in, strlength + len + 1);
    if (!tmp)
    {
      exit(EXIT_FAILURE);
    }
    in = tmp;
    strlength += len;
    strcat(in, "やったぜ。");
  }
  /*
   * 文字コード変換
   */
  charset charset = UTF_8;
  char *out = NULL;
  char encode = 0;

  switch (charset)
  {
  case UTF_8:
    //  エンコード用変数にそのまま代入
    // freeするために複製を入れる
    out = strdup(in);
    encode = 0;
    break;
  case UNICODE:
    //  文字コードを変換してから代入
    encode_utf8_2_unicode(&out, in);
    encode = 1;
    break;
  case SHIFT_JIS:
    //  文字コードを変換してから代入
    encode_utf8_2_sjis(&out, in);
    encode = 2;
    break;
  default:
    //  エンコード用変数にそのまま代入
    // freeするために複製を入れる
    out = strdup(in);
    break;
  }

  free(in);
  if (out == NULL)
  {
    perror("out encode OR copy failed");
    return EXIT_FAILURE;
  }

  /*
   * TODO: encode関数に分離
   * 棒読みちゃん向けにエンコード
   */
  short command = 1;
  short speed = -1;
  short tone = -1;
  short volume = -1;
  short voice = 0;
  size_t length = strlen(out);
  fprintf(stderr, "length : %ld\n", length);

  // なぜhtonsなしで読み上げできるのか謎
  // 棒読みちゃんはリトルエンディアン指定だそうです
  // c#サンプルでBinaryWriterを使ってたから本体でもBinaryReader使ってるんじゃないんですか？知らんけど

  size_t send_len = 15 + length;
  char *send_buf = malloc(send_len);
  *((short *)&send_buf[0]) = command;
  *((short *)&send_buf[2]) = speed;
  *((short *)&send_buf[4]) = tone;
  *((short *)&send_buf[6]) = volume;
  *((short *)&send_buf[8]) = voice;
  *((char *)&send_buf[10]) = encode;
  *((int *)&send_buf[11]) = length;
  memcpy(send_buf + 15, out, length);
  free(out);

  char servAddr[NI_MAXHOST] = ONION_SERV_ADDRESS;
  char servPortStr[NI_MAXSERV] = DEFAULT_PORT_STR;
  int use_onion = 0;
  if (use_onion == 1)
  {
    strncpy(servAddr, ONION_SERV_ADDRESS, NI_MAXHOST);
  }
  else
  {
    strncpy(servAddr, DEFAULT_SERV_ADDRESS, NI_MAXHOST);
  }
  rc = send_to_server(servAddr, servPortStr, send_buf, send_len);
  free(send_buf);
  return rc;
}
