/**
 * 今回: BouyomiChanSample.cpp
 * 前回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
 * 前前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
 */

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
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    short speed = -1, tone = -1, volume = -1, voice = 0;
    const char *msg;

    //コマンドライン引数処理
    switch (argc)
    {
    case 1:
        msg = "テストでーす";
        break;
    case 2:
        msg = argv[1];
        break;
    case 6:
        speed = atoi(argv[1]);
        tone = atoi(argv[2]);
        volume = atoi(argv[3]);
        voice = atoi(argv[4]);
        msg = argv[5];
        break;
    default:
        printf("使用法1>%s 文章\n", argv[0]);
        printf("使用法2>%s 速度(50-300) 音程(50-200) 音量(0-100) 声質(0-8) "
               "文章\n",
               argv[0]);
        return -1;
    }
    size_t len = strlen(msg);

    unsigned char header[15];
    //送信するデータの生成(文字列を除いた先頭の部分)
    *((short *)(header + 0)) = (short)htole16((unsigned short)0x0001);
    *((short *)(header + 2)) = (short)htole16((unsigned short)speed);
    *((short *)(header + 4)) = (short)htole16((unsigned short)tone);
    *((short *)(header + 6)) = (short)htole16((unsigned short)volume);
    *((short *)(header + 8)) = (short)htole16((unsigned short)voice);
    *((char *)(header + 10)) = 0;
    *((int *)(header + 11)) = (int)htole32((unsigned int)len);

    char host[NI_MAXHOST] = "192.168.12.5";
    char port[NI_MAXSERV] = "50001";

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(host, port, &hints, &res);

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    send(sock, header, 15, 0);
    send(sock, msg, len, 0);

    close(sock);

    return 1;
}
