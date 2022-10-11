/**
 * 今回: BouyomiChanSample.cpp
 * 前回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
 * 前前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
 */

#include "yattaze.h"
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
 * ? : 0x0000
 * Talk:0x0001
 * Pause:0x0010
 * Resume:0x0020
 * Skip:0x0030
 * Clear:0x0040
 * GetPause:0x0110
 * GetNowPlaying:0x0120
 * GetTaskCount:0x0130
 * void Talk(speed, tone, volume, voice, encode, length, message);
 * unsigned char GetPause();
 * unsigned char GetNowPlaying();
 * unsigned int GetTaskCount();
 *
 * TODO: bouyomi コマンドのサブコマンドとして各種コマンドを実装する
 * 環境変数でサーバー情報を設定する https://12factor.net/ja/
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(const int argc, const char *argv[])
{
    short command = 0x0001, speed = -1, tone = -1, volume = -1, voice = 0;
    char code = 0;
    const char *host_msg = NULL;

    //コマンドライン引数処理
    switch (argc)
    {
    case 1:
        host_msg = YATTAZE;
        break;
    case 2:
        host_msg = argv[1];
        break;
    case 6:
        speed = (short)atoi(argv[1]);
        tone = (short)atoi(argv[2]);
        volume = (short)atoi(argv[3]);
        voice = (short)atoi(argv[4]);
        host_msg = argv[5];
        break;
    default:
        printf("使用法1>%s\n", argv[0]);
        printf("使用法2>%s 文章\n", argv[0]);
        printf("使用法3>%s 速度(50-300) 音程(50-200) 音量(0-100) 声質(0-8) "
               "文章\n",
               argv[0]);
        return -1;
    }
    // TODO: encodingを変換するならここ
    const char *encoded_message = host_msg;
    size_t len = strlen(encoded_message);

    char name[NI_MAXHOST] = "192.168.12.5";
    char service[NI_MAXSERV] = "50001";

    unsigned char header[15];
    // 送信するデータの生成(文字列を除いた先頭の部分)
    *((short *)(header + 0)) = (short)htole16((unsigned short)command);
    *((short *)(header + 2)) = (short)htole16((unsigned short)speed);
    *((short *)(header + 4)) = (short)htole16((unsigned short)tone);
    *((short *)(header + 6)) = (short)htole16((unsigned short)volume);
    *((short *)(header + 8)) = (short)htole16((unsigned short)voice);
    *((char *)(header + 10)) = code;
    *((int *)(header + 11)) = (int)htole32((unsigned int)len);

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    hints.ai_socktype = SOCK_STREAM;

    int err = 0;
    if ((err = getaddrinfo(name, service, &hints, &res)) != 0)
    {
        fprintf(stderr, "%s", gai_strerror(err));
        return 1;
    }

    int ret = 0;
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0)
    {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("connect");
        close(sock);
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);
    res = NULL;

    if (send(sock, header, 15, 0) < 0)
    {
        perror("send 1");
        ret = 1;
        goto fail;
    }
    if (send(sock, host_msg, len, 0) < 0)
    {
        ret = 1;
        perror("send 2");
        goto fail;
    }

fail:

    if (close(sock) < 0)
    {
        ret = 1;
    }
    sock = -1;

    return ret;
}
