
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

// proxyアドレス
#define PROXY_URL "localhost"
#define PROXY_PORT 9050

#define BUF_SIZE 8192

typedef struct url_t{
	char host[BUF_SIZE];
	char path[BUF_SIZE];
	char query[BUF_SIZE];
	char fragment[BUF_SIZE];
	unsigned short port;
} URL;

int main(int argc, char** argv){

	int s = 0;

	struct addrinfo hints, *res = NULL, *ai;
	int err;

	char send_buf[BUF_SIZE];
	URL url = {
		"www.nicovideo.jp",
		"/",
		"",
		"",
		80
	};

	printf("get http://%s%s%s\n", url.host, url.path, url.query);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	printf("%s\n", url.host);
	//
	if((err = getaddrinfo(url.host, "http", &hints, &res))!=0){
		fprintf(stderr, "error : %s\n", gai_strerror(err));
		perror("getaddrinfo");
		return 1;
	}

	char str[INET6_ADDRSTRLEN];
	int type = 0;
	memset(str, 0, INET6_ADDRSTRLEN);

	for(ai = res; ai; ai = ai->ai_next){
		s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if(s < 0){
			perror("socket");
			return 1;
		}
		inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, str, INET6_ADDRSTRLEN);
		printf("%s\n", str);

		if(connect(s, ai->ai_addr, ai->ai_addrlen) != 0){
			perror("connect");
			close(s);
			s = -1;
			continue;
		}
		puts("connected");
		break;
	}
	//sockaddr_print("sockaddr is ", ai->ai_addr, ai->ai_addrlen);
	freeaddrinfo(res);

	size_t send_size = 0;
	sprintf(send_buf, "GET %s HTTP/1.1\r\n", url.path);
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	sprintf(send_buf + strlen(send_buf), "Host: %s\r\n", url.host);
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	sprintf(send_buf + strlen(send_buf), "Connection: close\r\n");
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	sprintf(send_buf + strlen(send_buf), "User-Agent: %s\r\n", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36");
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	sprintf(send_buf + strlen(send_buf), "Accept: %s\r\n", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	//sprintf(send_buf + strlen(send_buf), "Accept-Encoding: %s\r\n", "");
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	sprintf(send_buf + strlen(send_buf), "Accept-Language: %s\r\n", "ja, en-US, en");
	//send_size = write(s, send_buf, strlen(send_buf));
	//printf(send_buf);
	//printf("send %ld\n", send_size);

	sprintf(send_buf + strlen(send_buf), "\r\n");
	send_size = write(s, send_buf, strlen(send_buf));
	printf("send %ld\n", send_size);
	printf("%s\n", send_buf);

	char recv_buf[BUF_SIZE];
	size_t read_size = 0;
	while(1){
		read_size = read(s, recv_buf, read_size);
		printf("%ldbytes readed\n", read_size);
		if(read_size > 0){
			send_size = write(1, recv_buf, read_size);
			printf("write to stdout %ldbytes\n", send_size);
		} else {
			break;
		}
	}

	printf("errno : %d\n", errno);
	close(s);
	return 0;	
}

