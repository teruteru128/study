
#ifndef BM_H
#define BM_H

#include <openssl/ripemd.h>
#include <openssl/sha.h>

typedef struct
{
    SHA512_CTX sha512ctx;
    RIPEMD160_CTX ripemd160ctx;
} RIPE_CTX;

/* int ripe(ripectx, signpub, encpub) */
int ripe(RIPE_CTX *, char *signpub, size_t offsetsingpub, size_t lengthsignpub, char *encpub, size_t offsetencpub, size_t lengthencpub);

#endif
