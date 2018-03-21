
#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <string.h>
//#include <ctype.h>
//#include <inttypes.h>
#include <openssl/sha.h>

#define BUFFER_SIZE 1024

static void safe_free(void* ptr){
    if(ptr != NULL){
        free(ptr);
    }
}

static void pt(unsigned char* md)
{
	int i;

	for (i=0; i<SHA_DIGEST_LENGTH; i++)
		printf("%02x",md[i]);
	printf("\n");
}

int main(int argc, char** argv) {
    SHA_CTX* c = NULL;
    char *pubKey = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    size_t pubKey_len = 0;
    char* verifire = "10895332777036447559";
    size_t verifire_len = 0;
    unsigned char md[SHA_DIGEST_LENGTH];
    unsigned char buf[BUFFER_SIZE];

    pubKey_len = strlen(pubKey);
    verifire_len = strlen(verifire);

    c = malloc(sizeof(SHA_CTX));
    if(c == NULL) {
        goto done;
    }

    SHA1_Init(c);
    SHA1_Update(c, pubKey, pubKey_len);
    SHA1_Update(c, verifire, verifire_len);
    SHA1_Final(md, c);

    pt(md);

    done:
    safe_free(c);
    return 0;
}
