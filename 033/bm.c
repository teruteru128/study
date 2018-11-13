
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()
#include "bitmessage.h"
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442

int msgsend(int sock, char* msg){
	return write(sock, msg, strlen(msg));
}

int main(int argc, char* argv[]){
	int sock;
	struct sockaddr_in servSockAddr;
	unsigned short servPort;
	printf("%d\n", BM_init());

	memset(&servSockAddr, 0, sizeof(servSockAddr));
	servSockAddr.sin_family = AF_INET;

	if(inet_aton(localhost_ip, &servSockAddr.sin_addr) == 0){
		fprintf(stderr, "\n");
		perror("inet_aton");
		return EXIT_FAILURE;
	}
	servSockAddr.sin_port = htons(bitmessage_port);

	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket");
		return EXIT_FAILURE;
	}

	if (connect(sock, (struct sockaddr*) &servSockAddr, sizeof(servSockAddr)) < 0) {
		perror("connect() failed.");
		exit(EXIT_FAILURE);
	}
	printf("connected.\n");
	ssize_t w = 0;
	msgsend(sock, "POST / HTTP/1.0\r\n");
	msgsend(sock, "Authorization: Basic dGVydXRlcnUxMjg6Xjs9NVwke0JxTzppdF5ranV3WEU4ZkMuVS0hWylJXUBM\r\n");
	//msgsend(sock, "HOST: localhost\r\n");
	//msgsend(sock, "Connection: close\r\n");
	msgsend(sock, "\r\n");
	msgsend(sock, "<?xml version=\"1.0\"?>\r\n");
	msgsend(sock, "<methodCall>\r\n");
	msgsend(sock, "  <methodName>helloWorld</methodName>\r\n");
	msgsend(sock, "  <params>\r\n");
	msgsend(sock, "    <param>\r\n");
	msgsend(sock, "      <value>\r\n");
	msgsend(sock, "        <struct>\r\n");
	msgsend(sock, "          <member>\r\n");
	msgsend(sock, "            <name>firstWord</name>\r\n");
	msgsend(sock, "            <value><string>123</string></value>\r\n");
	msgsend(sock, "          </member>\r\n");
	msgsend(sock, "          <member>\r\n");
	msgsend(sock, "            <name>secondWord</name>\r\n");
	msgsend(sock, "            <value><string>456</string></value>\r\n");
	msgsend(sock, "          </member>\r\n");
	msgsend(sock, "        </struct>\r\n");
	msgsend(sock, "      </value>\r\n");
	msgsend(sock, "    </param>\r\n");
	msgsend(sock, "  </params>\r\n");
	msgsend(sock, "</methodCall>\r\n");
	msgsend(sock, "\r\n");
	int emptyLineCount = 0;
	size_t msgsize = 1024;
	size_t bufsize = msgsize + 1;
	char buf[1025];
	size_t len = 0;
	
	while((len = read(sock, buf, bufsize)) > 0){
		printf("%s\n", buf);
	}
	/*
	 *<?xml version="1.0"?>
	 * <methodCall>
	 *  <methodName>examples.getStateName</methodName>
	 *  <params>
	 *    <param>
	 *     <value><i4>40</i4></value>
	 *    </param>
	 *  </params>
	 *</methodCall>
	 * */
	close(sock);
	return EXIT_SUCCESS;
}
