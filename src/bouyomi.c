/*
 * 今回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
 * 前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
 */

#include "config.h"
#include <stddef.h>
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

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h> //close()
#endif

#include <endian.h>

#define YATTAZE "やったぜ。"

int encodeHeader(char header[15], char encode, size_t length)
{
  short command = 1;
  short speed = -1;
  short tone = -1;
  short volume = -1;
  short voice = 0;
#ifdef _DEBUG
  fprintf(stderr, "length : %ld\n", length);
#endif
  if (length > INT_MAX)
  {
    fputs("読み上げ文書が長すぎます。\n", stderr);
    return EXIT_FAILURE;
  }
  *((short *)(header + 0)) = (short)htole16((unsigned short)command);
  *((short *)(header + 2)) = (short)htole16((unsigned short)speed);
  *((short *)(header + 4)) = (short)htole16((unsigned short)tone);
  *((short *)(header + 6)) = (short)htole16((unsigned short)volume);
  *((short *)(header + 8)) = (short)htole16((unsigned short)voice);
  *((char *)(header + 10)) = encode;
  *((int *)(header + 11)) = (int)htole32((unsigned int)length);
  return EXIT_SUCCESS;
}

int sendToServer(char *host, char *port, char header[15], char *msg, size_t length)
{
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  int rc = getaddrinfo(host, port, &hints, &res);
  if (rc != 0)
  {
    fprintf(stderr, "getaddrinfo : [%s]:%s %s\n", host, port, gai_strerror(rc));
    return rc;
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
#ifdef _DEBUG
    print_addrinfo0(adrinf, stderr);
#endif
    break;
  }
  freeaddrinfo(res);
  if (rc < 0)
  {
    perror("connect()");
    close(sock);
    return rc;
  }

  ssize_t w = write(sock, header, 15);
  // 送信
  if (w != 15)
  {
    fprintf(stderr, _("Error! %s\n"), strerror(errno));
    rc = close(sock);
    perror("write header");
    return rc;
  }
  w = write(sock, msg, length);
  if (w != (ssize_t)length)
  {
    perror("Error!");
    rc = close(sock);
    perror("write header");
    return rc;
  }
  fprintf(stderr, _("Sent!\n"));
  // ソケットを閉じる
  rc = close(sock);
  if (rc != 0)
  {
    perror("close");
    return rc;
  }
  return 0;
}

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
  char host[NI_MAXHOST] = DEFAULT_SERV_ADDRESS_2;
  char port[NI_MAXSERV] = DEFAULT_PORT_STR;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--host") == 0 && (i + 1) < argc)
    {
      strncpy(host, argv[i + 1], NI_MAXHOST - 1);
      i++;
      continue;
    }
    if (strcmp(argv[i], "--port") == 0 && (i + 1) < argc)
    {
      strncpy(port, argv[i + 1], NI_MAXSERV - 1);
      i++;
      continue;
    }
    if ((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0) && (i + 1) < argc)
    {
      i++;
      continue;
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
    errno = 0;
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
  // 縮小
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
   * ホストの文字コードをUTF-8に仮定していいんだろうか
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

  size_t length = strlen(out);

  // なぜhtonsなしで読み上げできるのか謎
  // 棒読みちゃんはリトルエンディアン指定だそうです
  // c#サンプルでBinaryWriterを使ってたから
  // 本体でもBinaryReader使ってるんじゃないんですか？
  // 知らんけど
  // ヘッダー長が8の倍数じゃないのつらい

  char header[15];
  if (encodeHeader(header, encode, length) != EXIT_SUCCESS)
  {
    free(out);
    return EXIT_FAILURE;
  }

  rc = sendToServer(host, port, header, out, length);
  free(out);
  return rc;
}
