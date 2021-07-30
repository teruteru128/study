/*
 * 今回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
 * 前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <libintl.h>
#include <locale.h>
#include <stddef.h>
#define _(str) gettext(str)
#include "bouyomi.h"
#include <charset-convert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <print_addrinfo.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <sys/types.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
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
#define BOUYOMI_HEADER_SIZE 15

void encode_talk_header1(unsigned char *header, short speed,
                    short tone, short volume, short voice, char encode,
                    int length)
{
    *((short *)(header + 0)) = (short)htole16((unsigned short)0x0001);
    *((short *)(header + 2)) = (short)htole16((unsigned short)speed);
    *((short *)(header + 4)) = (short)htole16((unsigned short)tone);
    *((short *)(header + 6)) = (short)htole16((unsigned short)volume);
    *((short *)(header + 8)) = (short)htole16((unsigned short)voice);
    *((char *)(header + 10)) = encode;
    *((int *)(header + 11)) = (int)htole32((unsigned int)length);
}

void encode_talk_header(unsigned char *header, char encode, int length)
{
    encode_talk_header1(header, -1, -1, -1, 0, 2, length);
}

void parsearg(struct args *args, int argc, char *argv[])
{

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--host") == 0)
            && (i + 1) < argc)
        {
            strncpy(args->servHost, argv[i + 1], NI_MAXHOST - 1);
            i++;
            continue;
        }
        if ((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
            && (i + 1) < argc)
        {
            strncpy(args->servPort, argv[i + 1], NI_MAXSERV - 1);
            i++;
            continue;
        }
        if ((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0)
            && (i + 1) < argc)
        {
            i++;
            continue;
        }
    }
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
 * proxyは外部で対処 torsocksとか
 *
 * メッセージが指定されていないときはメッセージを指定して終了？
 * もしくは標準入力で入力できるようにする？
 *
 * https://github.com/torproject/tor/blob/03867b3dd71f765e5adb620095692cb41798c273/src/app/config/config.c#L2537
 * parsed_cmdline_t* config_parse_commandline(int argc, char **argv, int
 * ignore_errors) 引数を何も指定しないときはヘルプを表示して終了？
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
 * 各種コマンド
 * Talk:0x0001
 * Pause:0x0010
 * Resume:0x0020
 * Skip:0x0030
 * GetPause:0x0110
 * GetNowPlaying:0x0120
 * GetTaskCount:0x0130
 * void Talk(speed, tone, volume, voice, encode, length, message);
 * unsigned char GetPause();
 * unsigned char GetNowPlaying();
 * unsigned int GetTaskCount();
 */
int main(int argc, char *argv[])
{
    int rc = 0;

    // gettextを初期化する
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

    // コマンドライン引数をパースする
    struct args args = { 0, 0, DEFAULT_SERV_ADDRESS_2, DEFAULT_PORT_STR };
    parsearg(&args, argc, argv);

    // 読み上げメッセージを読み込む
    // 文字コード変換前読み上げ文書
    char *reading_message_before_encode = malloc(BUFSIZ);
    if (!reading_message_before_encode)
    {
        perror("malloc, reading_message_before_encode");
        return EXIT_FAILURE;
    }
    // memset()で初期化するところを最初の文字だけ消して代用
    reading_message_before_encode[0] = 0;
    char *realloctmp = NULL;
    size_t strlength = 0;
    size_t len = 0;
    size_t capacity = BUFSIZ;
    char buf[BUFSIZ];
    size_t minnewcapa = 0;
    // VS Codeのターミナルは端末じゃなくて何なんだろうな？
    // 標準入力から読み上げ文書読み込み
    while (fgets(buf, BUFSIZ, stdin) != NULL)
    {
        len = strlen(buf);
        if (len == 0)
        {
            // 無限ループの可能性……？
            continue;
        }
        // strncatするのに最低限必要なメモリ領域サイズ
        minnewcapa = strlength + len + 1;
        if (minnewcapa > capacity)
        {
            // 領域サイズが足りないときは拡張
            while (minnewcapa > capacity)
            {
                capacity *= 2;
            }
            realloctmp = realloc(reading_message_before_encode, capacity);
            if (!realloctmp)
            {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            reading_message_before_encode = realloctmp;
        }
        // 連結
        strncat(reading_message_before_encode + strlength, buf,
                capacity - (strlength + 1));
        // 読み込み済み文字列長
        strlength += len;
    }
    // メッセージが空ならデフォルトメッセージを読ませる
    if (strlength == 0)
    {
        // ここまでに入力がされなかった

        // デフォルトのメッセージを読ませる
        len = strlen(YATTAZE);
        strlength += len;
        strncat(reading_message_before_encode, YATTAZE, capacity);
    }

    // テキストバッファ縮小(最適化)
    capacity = strlength + 1;
    realloctmp = realloc(reading_message_before_encode, capacity);
    if (realloctmp == NULL)
    {
        perror("realloc(strlen(reading_message_before_encode) + 1)");
        return EXIT_FAILURE;
    }
    else
    {
        reading_message_before_encode = realloctmp;
    }

    /*
     * 文字コード変換
     */
    // ホストの文字コードをUTF-8に仮定していいんだろうか
    charset_t charset = UTF_8;
    char *reading_message_after_encode = NULL;
    char encode = 0;

    switch (charset)
    {
    case UTF_8:
        //  エンコード用変数にそのまま代入
        reading_message_after_encode = reading_message_before_encode;
        encode = 0;
        break;
    case UNICODE:
        //  文字コードを変換してから代入
        encode_utf8_2_unicode(&reading_message_after_encode,
                              reading_message_before_encode);
        encode = 1;
        break;
    case SHIFT_JIS:
        //  文字コードを変換してから代入
        encode_utf8_2_sjis(&reading_message_after_encode,
                           reading_message_before_encode);
        encode = 2;
        break;
    default:
        //  エンコード用変数にそのまま代入
        reading_message_after_encode = reading_message_before_encode;
        break;
    }

    if (reading_message_before_encode != reading_message_after_encode)
        free(reading_message_before_encode);
    if (reading_message_after_encode == NULL)
    {
        perror("reading_message_after_encode encode OR copy failed");
        return EXIT_FAILURE;
    }

    size_t length = strlen(reading_message_after_encode);

    // ヘッダーをエンコード
    // なぜhtonsなしで読み上げできるのか謎->棒読みちゃんはリトルエンディアン指定だそうです
    // c#サンプルでBinaryWriterを使ってたから
    // 本体でもBinaryReader使ってるんじゃないんですか？
    // 知らんけど
    // ヘッダー長が8の倍数じゃないのどうなんですかね？
    // アライメントされてないのが辛いんだよ

    unsigned char header[BOUYOMI_HEADER_SIZE];
#ifdef _DEBUG
    fprintf(stderr, "length : %ld\n", length);
#endif
    if (length > INT_MAX)
    {
        fputs("読み上げ文書が長すぎます。\n", stderr);
        return EXIT_FAILURE;
    }
    encode_talk_header(header, encode, (int)length);

    // 棒読みちゃんサーバーに接続して送信
    // ホストとサービスを変換して接続
    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    hints.ai_socktype = SOCK_STREAM;
    rc = getaddrinfo(args.servHost, args.servPort, &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo : [%s]:%s %s\n", args.servHost,
                args.servPort, gai_strerror(rc));
        return rc;
    }

    struct addrinfo *adrinf = NULL;
    int sock = 0;
    for (adrinf = res; adrinf != NULL; adrinf = adrinf->ai_next)
    {
        sock = socket(adrinf->ai_family, adrinf->ai_socktype,
                      adrinf->ai_protocol);
        if (sock < 0)
        {
            perror("socket()");
            continue;
        }
        rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
        if (rc < 0)
        {
            perror("connect");
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
        return EXIT_FAILURE;
    }

    // 送信
    ssize_t w = write(sock, header, BOUYOMI_HEADER_SIZE);
    if (w != BOUYOMI_HEADER_SIZE)
    {
        perror("write header");
        close(sock);
        return EXIT_FAILURE;
    }
    w = write(sock, reading_message_after_encode, length);
    if (w != (ssize_t)length)
    {
        perror("write header");
        close(sock);
        return EXIT_FAILURE;
    }
    fprintf(stderr, _("Sent!\n"));
    // ソケットを閉じる
    rc = close(sock);
    if (rc != 0)
    {
        perror("close");
        return EXIT_FAILURE;
    }
    free(reading_message_after_encode);
    return EXIT_SUCCESS;
}
