
#include <stdio.h>

#define h0 0x67452301
#define h1 0xEFCDAB89
#define h2 0x98BADCFE
#define h3 0x10325476
#define h4 0xC3D2E1F0

#define SHA_LONG unsigned int

#define SHA_LBLOCK 16
#define SHA_CBLOCK (SHA_LBLOCK*4)
#define SHA_LAST_BLOCK (SHA_CBLOCK-8)
#define SHA_DIGEST_LENGTH 20

typedef struct SHAstate_st{
    SHA_LONG s0,s1,s2,s3,s4;
    SHA_LONG Nl, Nh;
    SHA_LONG data[SHA_LBLOCK];
}SHA_CTX;

void sha_init(SHA_CTX *c){

}
void sha_update(SHA_CTX *c, const void* data, const size_t len){

}

void sha_finish(unsigned char *md, SHA_CTX *c){

}

