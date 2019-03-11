#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

// http://developer.wonderpla.net/entry/blog/engineer/network_program_with_cpp_01/
int main(int argc, char* argv[]){
  unsigned int port = 0;
  const char* ip_address = "192.168.1.3";
  struct sockaddr_in addr = {0, };

  int sock = 0;
  for(port = 1; port < 65536; port++){
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0){
      perror("socket");
      return EXIT_FAILURE;
    }
    printf("%5d, ", port);
    /* 接続のために構造体に値を入れる */
    addr.sin_family = AF_INET;
    /* 接続先のポート番号。ビッグエンディアンの32ビットに変換*/
    addr.sin_port = htons(port);
    /* inet_addrはxxx.xxx.xxx.xxxの文字列をビッグエンディアン(ネットワークバイトオーダー)の32ビットに変換する。*/
    addr.sin_addr.s_addr = inet_addr(ip_address);

    /* サーバにコネクトを行う。この関数はブロック型である。
       　 コネクトのタイムアウトはデバイスドライバーの実装によって違う。
       このタイムアウト値の設定もくせものでOSによっては設定ができない。
       なので、タイムアウトをさせる場合は、別途実装する必要があるが、色々と説明が長くなるので
       ライブラリにまとめる際のコードを参考にして欲しい。 

       第二引数のstruct sockaddrのキャストだが、sockの第一引数に何を指定しているかで指定される構造体が違うためにキャストすることになる。
       現代のソフトウェア工学的には気持ち悪い実装かも知れないが、connectがシステムコールである故にやむを得ないところがある。
       */
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr))!=0)
    {
      /* 失敗*/
      printf("socket connect error!!! (%u, %d)\n", port, errno);
      //return EXIT_FAILURE;
    } else {
      printf("OK(%u)\n", port);
    }
    close(sock);
  }
  return EXIT_SUCCESS;
}
