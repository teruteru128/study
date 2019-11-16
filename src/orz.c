
#include <stdio.h>
#include <err.h>
#include <stdint.h>
#include <string.h>

#define URANDOM_PATH "/dev/urandom"
#define BUF_SIZE 8
#include "random.h"

static const char* hexchars = "0123456789abcdef";
static void hex_dump(const void* pt, const size_t len){
	char* p = (char *)pt;
	int i = 0;
	for(i = 0; i < len; i++){
		printf("%c%c", hexchars[(p[i] >> 4) & 0x0F], hexchars[(p[i] >> 0) & 0x0F]);
		if( i % 16 == 15){
			printf("\n");
		}
	}
}

int main(int argc, char** argv){
	char buf[BUF_SIZE];
	uint32_t n[BUF_SIZE / sizeof(uint32_t)];
	int len = 4;
	// /dev/urandom から8192バイトも読み込むことないよね？
	if(get_random(URANDOM_PATH, buf, sizeof(char), BUF_SIZE) != 0){
		warnx("failed");
		return -1;
	}
	//hex_dump(buf, sizeof(buf));
	// byte配列を整数に変換する
	memcpy(n, (char *)buf, BUF_SIZE);
	//hex_dump(n, sizeof(n));
	size_t max = BUF_SIZE / sizeof(uint32_t), i = 0;

	uint32_t seed = n[0];
	char* messages[] = {
		"orz",
    NULL
	};
  size_t messages_size = 0;
  char** tmp = messages;
  while(*tmp++ != NULL){
    messages_size++;
  }
	for(i = 0; i < 334;i++){
		printf("%s\n", messages[(seed = xor(seed)) % messages_size]);
	}

	return 0;
}
