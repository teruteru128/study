/*
    今回:https://qiita.com/tajima_taso/items/13b5662aca1f68fc6e8e
    前回:https://qiita.com/tajima_taso/items/fb5669ddca6e4d022c15
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in servSockAddr;
    unsigned short servPort;
    char recvBuffer[BUFSIZE];
    char sendBuffer[BUFSIZE];

    if (argc != 3) {
            fprintf(stderr, "argument count mismatch error.\n");
            exit(EXIT_FAILURE);
    }

        memset(&servSockAddr, 0, sizeof(servSockAddr));

        servSockAddr.sin_family = AF_INET;

	// IPアドレスをバイナリに変換
    if (inet_aton(argv[1], &servSockAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP Address.\n");
        exit(EXIT_FAILURE);
    }

    if ((servPort = (unsigned short) atoi(argv[2])) == 0) {
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

    while(1){
        printf("please enter the characters:");
        if (fgets(sendBuffer, BUFSIZE, stdin) == NULL) {
            fprintf(stderr, "invalid input string.\n");
            exit(EXIT_FAILURE);
        }

    if (send(sock, sendBuffer, strlen(sendBuffer), 0) <= 0) {
        perror("send() failed.");
        exit(EXIT_FAILURE);
    }

    int byteRcvd  = 0;
    int byteIndex = 0;
    while (byteIndex < MSGSIZE) {
        byteRcvd = recv(sock, &recvBuffer[byteIndex], 1, 0);
        if (byteRcvd > 0) {
            if (recvBuffer[byteIndex] == '\n') {
                recvBuffer[byteIndex] = '\0';
                if (strcmp(recvBuffer, "quit") == 0) {
                    close(sock);
                    return EXIT_SUCCESS;
                } else {
                        break;
                }
            }
            byteIndex += byteRcvd;
         } else if(byteRcvd == 0) {
            perror("ERR_EMPTY_RESPONSE");
            exit(EXIT_FAILURE);
         } else {
            perror("recv() failed.");
            exit(EXIT_FAILURE);
         }
     }
        printf("server return: %s\n", recvBuffer);
  }

    return EXIT_SUCCESS;
}
