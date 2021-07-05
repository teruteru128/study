
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>

#define BIG_PRECURE_IS_WATCHING_YOU "BIG PRECURE IS WATCHING YOU"

int main(int argc, char const *argv[])
{
    const EVP_MD *sha512 = EVP_sha512();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, sha512);
    EVP_DigestUpdate(ctx, BIG_PRECURE_IS_WATCHING_YOU, strlen(BIG_PRECURE_IS_WATCHING_YOU));
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    EVP_DigestFinal(ctx, md, &len);
    for (unsigned int i = 0; i < len; i++)
    {
        printf("%02x", md[i]);
    }
    printf("\n");
    EVP_DigestInit(ctx, sha512);
    EVP_DigestUpdate(ctx, md, len);
    EVP_DigestFinal(ctx, md, &len);
    for (unsigned int i = 0; i < len; i++)
    {
        printf("%02x", md[i]);
    }
    printf("\n");
    EVP_MD_CTX_free(ctx);
    return 0;
}
