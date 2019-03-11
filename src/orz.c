
#include <stdio.h>
#include <err.h>
#include <stdint.h>
#include <string.h>

#define URANDOM_PATH "/dev/urandom"
#define BUF_SIZE 8

static int get_random(const char* const path, char* buf, const size_t size, const size_t nmemb) {

	FILE* rnd = fopen(path, "rb");
	if(rnd == NULL){
		perror("fopen rnd");
		return -1;
	}
	size_t r = fread(buf, size, nmemb, rnd);
	if(r < 0){
		perror("fread rnd");
		return -1;
	}
	if(r != (size * nmemb)){
		warnx("reading failed");
	}

	(void) fclose(rnd);
	return 0;
}
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
static uint32_t xor(const uint32_t seed){
	uint32_t s = seed;
	s = s ^ (s << 13);
	s = s ^ (s >> 17);
	return s ^ (s << 15);
}
static uint64_t xor64(const uint64_t seed){
	uint64_t s = seed;
	s = s ^ (s << 13);
	s = s ^ (s >> 7);
	return s ^ (s << 17);
}
static uint32_t xor96(const uint32_t seed1, const uint32_t seed2, const uint32_t seed3){
	return 0;
}
static uint32_t xor128(const uint32_t seed1, const uint32_t seed2, const uint32_t seed3, const uint32_t seed4){
	return 0;
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
		"失礼しました。熱盛と出てしまいました。",
		"http://games.kids.yahoo.co.jp/sports/013.html",
		"http://games.kids.yahoo.co.jp/sports/015.html",
		"http://www.nicovideo.jp/watch/sm20465807",
		"やらないか。",
		"はずれ",
		"ンアッー！",
		"スカ",
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
