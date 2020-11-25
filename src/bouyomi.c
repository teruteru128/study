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
#include <err.h>
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

#include <endian.h>

#define YATTAZE "やったぜ。"

/**
 * @brief 
 * 
 * 1. コマンドライン引数解析
 * 2. 読み上げ文書をコマンドライン引数もしくは標準入力からメモリへ展開
 * 2. 読み上げ文書文字コード変換
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
 * --host
 *   デフォルト localhost
 * --port
 *   デフォルト 50001
 * charset
 * proxyは外部で対処
 *
 * https://github.com/torproject/tor/blob/03867b3dd71f765e5adb620095692cb41798c273/src/app/config/config.c#L2537
 * parsed_cmdline_t* config_parse_commandline(int argc, char **argv, int ignore_errors)
 * 引数を何も指定しないときはヘルプを表示して終了？
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
 *
 * bouyomic *bouyomi_client_new();
 *
 * ログレベルはグローバル領域においておかないと使いづらくないか？
 */
int main(int argc, char *argv[])
{
  int rc = 0;
  int use_onion = 0;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--use-onion"))
    {
      use_onion = 1;
    }
  }

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
  int ignore_errors = 0;
  struct args args;
  // 文字コード変換前読み上げ文書
  char *in = malloc(BUFSIZ);
  if (!in)
  {
    perror("malloc, in");
    return EXIT_FAILURE;
  }
  // memset()で初期化するところを最初の文字だけ消して代用
  in[0] = 0;
  char *realloctmp = NULL;
  size_t strlength = 0;
  size_t len = 0;
  size_t capacity = BUFSIZ;
  char buf[BUFSIZ];
  if (!isatty(fileno(stdin)))
  {
    // 標準入力(端末以外)から読み上げ文書読み込み
    while (fgets(buf, BUFSIZ, stdin) != NULL)
    {
      len = strlen(buf);
      if (len == 0)
      {
        // 無限ループの可能性……？
        continue;
      }
      // strncatするのに最低限必要なメモリ領域サイズ
      size_t minnewcapa = strlength + len + 1;
      if (minnewcapa > capacity)
      {
        // 領域サイズが足りないときは拡張
        while (minnewcapa > capacity)
        {
          capacity *= 2;
        }
        realloctmp = realloc(in, capacity);
        if (!realloctmp)
        {
          perror("realloc");
          exit(EXIT_FAILURE);
        }
        in = realloctmp;
      }
      // 連結
      strncat(in + strlength, buf, capacity - (strlength + 1));
      // 読み込み済み文字列長
      strlength += len;
    }
  }
  if (strlength == 0)
  {
    // 標準入力から読み上げ文書の入力がなかった
    len = strlen(YATTAZE);
#if 0
    // strlength == 0 をチェック済みなんだから足さなくてもいいんじゃないかな
    size_t reqlen = /* strlength + */ len + 1;
    if (reqlen > capacity)
    {
      // デフォルトのキャパシティが8192なので十中八九デッドコード
      realloctmp = realloc(in, reqlen);
      if (!realloctmp)
      {
        exit(EXIT_FAILURE);
      }
      in = realloctmp;
    }
#endif
    strlength += len;
    strncat(in, YATTAZE, capacity);
  }
  capacity = strlen(in) + 1;
  realloctmp = realloc(in, capacity);
  if (realloctmp == NULL)
  {
    perror("realloc(strlen(in) + 1)");
    return EXIT_FAILURE;
  }
  else
  {
    in = realloctmp;
  }
  /*
   * 文字コード変換
   */
  charset_t charset = UTF_8;
  char *out = NULL;
  char encode = 0;

  switch (charset)
  {
  case UTF_8:
    //  エンコード用変数にそのまま代入
    out = in;
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
    out = in;
    break;
  }

  if (in != out)
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
  if (length > INT_MAX)
  {
    free(out);
    fputs("読み上げ文書が長すぎます。\n", stderr);
    return EXIT_FAILURE;
  }

  // なぜhtonsなしで読み上げできるのか謎
  // 棒読みちゃんはリトルエンディアン指定だそうです
  // c#サンプルでBinaryWriterを使ってたから
  // 本体でもBinaryReader使ってるんじゃないんですか？
  // 知らんけど
  // ヘッダー長が8の倍数じゃないのつらい

  size_t send_len = 15 + length;
  char *send_buf = malloc(send_len);
  *((short *)(send_buf + 0)) = (short)htole16((unsigned short)command);
  *((short *)(send_buf + 2)) = (short)htole16((unsigned short)speed);
  *((short *)(send_buf + 4)) = (short)htole16((unsigned short)tone);
  *((short *)(send_buf + 6)) = (short)htole16((unsigned short)volume);
  *((short *)(send_buf + 8)) = (short)htole16((unsigned short)voice);
  *((char *)(send_buf + 10)) = encode;
  *((int *)(send_buf + 11)) = (int)htole32((unsigned int)length);
  memcpy(send_buf + 15, out, length);
  free(out);

  char servAddr[NI_MAXHOST];
  char servPortStr[NI_MAXSERV] = DEFAULT_PORT_STR;
  if (use_onion == 1)
  {
    strncpy(servAddr, ONION_SERV_ADDRESS, NI_MAXHOST);
  }
  else
  {
    strncpy(servAddr, DEFAULT_SERV_ADDRESS_2, NI_MAXHOST);
  }
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  rc = getaddrinfo(servAddr, servPortStr, &hints, &res);
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
      continue;
    }
    rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
    if (rc < 0)
    {
      close(sock);
      continue;
    }
    fprintf(stderr, "[DEBUG]:");
    print_addrinfo0(adrinf, stderr);
    break;
  }
  freeaddrinfo(res);
  if (rc < 0)
  {
    perror("connect()");
    close(sock);
    return EXIT_FAILURE;
  }

  ssize_t w = 0;
  // 送信
  if ((w = write(sock, send_buf, send_len)) <= 0)
  {
    fprintf(stderr, _("Error! %s\n"), strerror(errno));
  }
  else
  {
    fprintf(stderr, _("Sent!\n"));
  }
  // ソケットを閉じる
  rc = close(sock);
  if (rc != 0)
  {
    perror("close");
    return EXIT_FAILURE;
  }
  free(send_buf);
  return rc;
}
