/*
    今回: https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
    前回: https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
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

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)
#define DEFAULT_PORT 50001
#define DEFAULT_PORT_STR "50001"
#define DEFAULT_SERV_ADDRESS "localhost"
#define DEFAULT_SERV_ADDRESS4 "127.0.0.1"
#define DEFAULT_SERV_ADDRESS6 "::1"

/**
 * アドレスとポート番号を表示する。
 * <I> adrinf: アドレス情報
 */
void print_addrinfo(struct addrinfo *adrinf) {
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

  printf("[%s]:%s\n", hbuf, sbuf);
}


int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in servSockAddr;
    unsigned short servPort;
    char *servAddr = DEFAULT_SERV_ADDRESS;
    char *servPortStr = DEFAULT_PORT_STR;
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *adrinf = NULL;

    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];

    int rc = 0;

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

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    rc = getaddrinfo(servAddr, servPortStr, &hints, &res);
    if(rc!=0){
      perror("getaddrinfo");
    }

    for(adrinf = res; adrinf!=NULL;adrinf=adrinf->ai_next){
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
    char *mb = argv[1];

    // 棒読みちゃん向けにエンコード
    short command = 1;
    short speed = -1;
    short tone = -1;
    short volume = -1;
    short voice = 0;
    char encode = 0;
    size_t length = strlen(mb);
    size_t capacity = 0;

    // なぜhtonsなしで読み上げできるのか謎
    // 棒読みちゃんはリトルエンディアン指定だそうです
    //command = (short) htons((uint16_t)command);
    capacity += sizeof(command);

    //speed = (short) htons((uint16_t)speed);
    capacity += sizeof(speed);

    //tone = (short) htons((uint16_t)tone);
    capacity += sizeof(tone);

    //volume = (short) htons((uint16_t)volume);
    capacity += sizeof(volume);

    capacity += sizeof(encode);

    capacity += length;

    int32_t length_32 = (int32_t)length;
    //length_32 = (int) htonl((uint32_t)length);
    capacity += sizeof(length_32);

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
    printf("送信しました！\n");
    // ソケットを閉じる
    close(sock);
    return EXIT_SUCCESS;
}
