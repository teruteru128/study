/*
    今回:https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
    前回:https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()
#include <iconv.h>
#include <limits.h>
#include <locale.h>
#include <wchar.h>

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)
#define DEFAULT_PORT 50001
#define DEFAULT_PORT_STR "50001"
#define DEFAULT_SERV_ADDRESS "localhost"
#define DEFAULT_SERV_ADDRESS4 "127.0.0.1"
#define DEFAULT_SERV_ADDRESS6 "::1"

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in servSockAddr;
    unsigned short servPort;
    char servAddr[] = DEFAULT_SERV_ADDRESS4;
    char servPortStr[] = DEFAULT_PORT_STR;

    if(setlocale(LC_CTYPE, "") == NULL){
        perror("setlocale");
        return EXIT_FAILURE;
    }

    if (argc != 2) {
            fprintf(stderr, "argument count mismatch error.\n");
            exit(EXIT_FAILURE);
    }

    memset(&servSockAddr, 0, sizeof(servSockAddr));

    servSockAddr.sin_family = AF_INET;

    // IPアドレスをバイナリに変換
    if (inet_aton(servAddr, &servSockAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP Address.\n");
        exit(EXIT_FAILURE);
    }

    if ((servPort = (unsigned short) atoi(servPortStr)) == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }
    servSockAddr.sin_port = htons(servPort);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("socket() failed.");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*) &servSockAddr, sizeof(servSockAddr)) < 0) {
        perror("connect() failed.");
        exit(EXIT_FAILURE);
    }
    // コネクションの確立ここまで
    printf("connect to %s\n", inet_ntoa(servSockAddr.sin_addr));

    char *mb = argv[1];

    // 棒読みちゃん向けにエンコード
    size_t capacity = 0;
    short command = 1;
    short speed = -1;
    short tone = -1;
    short volume = -1;
    short voice = 0;
    char encode = 0;
    size_t length = strlen(mb);
    // なぜhtonsなしで読み上げできるのか謎
    //command = (short) htons(command);
    capacity += sizeof(command);

    //speed = (short) htons(speed);
    capacity += sizeof(speed);

    //tone = (short) htons(tone);
    capacity += sizeof(tone);

    //volume = (short) htons(volume);
    capacity += sizeof(volume);

    capacity += sizeof(encode);

    capacity += length;

    int length_32 = (uint32_t)length;
    //length_32 = (int) htonl((uint32_t)length);
    capacity += sizeof(length);

    ssize_t w = 0;
    // 送信
    w = write(sock, &command, sizeof(command));
    w = write(sock, &speed, sizeof(speed));
    w = write(sock, &tone, sizeof(tone));
    w = write(sock, &volume, sizeof(volume));
    w = write(sock, &voice, sizeof(voice));
    w = write(sock, &encode, sizeof(encode));
    w = write(sock, &length_32, sizeof(length_32));
    w = write(sock, mb, length);

    // ソケットを閉じる
    close(sock);
    return EXIT_SUCCESS;
}
