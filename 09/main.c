
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <openssl/sha.h>
#include "printint.h"

#define BUF_SIZE 1024

int main(int argc, char** argv) {
    const char *pubKey = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    SHA_CTX* c;
    int ret = EXIT_SUCCESS;
    char buf[BUF_SIZE];
    unsigned char md[SHA_DIGEST_LENGTH];
    uint64_t verifire = 0;
    unsigned int seed;
    size_t pubKeyLength = strlen(pubKey);
    size_t verifireLength;
    int i;

    //printf("OK\n");
    c = malloc(sizeof(SHA_CTX));
    if(c == NULL){
        ret = EXIT_FAILURE;
        goto done;
    }

    fgets(buf, BUF_SIZE, stdin);
    printf("%s",buf);
    if(strlen(buf) <= 0) {
        strncpy(buf, "0", 1);
    }
    seed = (unsigned int)(strtoul(buf, NULL, 10) & 0xFFFFFFFFUL);
    srandom(seed);
    verifire = (((uint64_t)random()) << 33) ^ (((uint64_t)random()) << 16) ^ ((uint64_t)random());
    printf("%"PRIu64", %"PRIx64"\n", verifire, verifire);
    for(;;verifire++){
        verifireLength = snprintUInt64(buf, BUF_SIZE, verifire);

        SHA1_Init(c);
        SHA1_Update(c, pubKey, pubKeyLength);
        SHA1_Update(c, buf, verifireLength);
        SHA1_Final(md, c);

        if(md[0] == 0 && md[1] == 0 && md[2] == 0 && md[3] == 0 && md[4] == 0){
            break;
        }
    }
    printf("END: %"PRIu64"\n", verifire);
    for(i = 0; i < SHA_DIGEST_LENGTH; i++){
        printf("%02x", md[i]);
    }
    printf("\n");

    done:
    free(c);
    if(ret == EXIT_FAILURE){
        perror(NULL);
    }
    printf("EXIT\n");
    return ret;
}
