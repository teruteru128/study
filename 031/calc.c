
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#define BUFSIZE (67108864)

//ファイルから先頭2行読んで16進数としてパースして掛け算する
//掛け算した結果のビット数を表示する
int main(int argc, char* argv[])
{
	if(argc < 2){
		fprintf(stderr, "usage:%s [path]\n", argv[0]);
		return 1;
	}

	FILE* fp = NULL;
	char* fname = argv[1];
	char* buf = calloc(BUFSIZE, sizeof(char));

	if(buf == NULL){
		perror("calloc");
		return 1;
	}

	size_t i;
	mpz_t num[2];
	mpz_t multply;

	mpz_inits(num[0], num[1], multply);

	fp = fopen(fname, "r");
	if(fp == NULL){
		perror("fopen");
		return 2;
	}
	for(i = 0; i < 2; i++){
		char *f = fgets(buf, BUFSIZE, fp);
		if(f == NULL){
			perror("fgets");
			fclose(fp);
			return 3;
		}
		mpz_set_str(num[i], f, 16);
	}
	mpz_mul(multply, num[0], num[1]);
	printf("%ldbits\n", mpz_sizeinbase(multply, 2));
	fclose(fp);
	free(buf);
	return 0;
}
