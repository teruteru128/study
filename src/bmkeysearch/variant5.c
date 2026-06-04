
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#include <assert.h>
#include <bitmessage.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define KEY_CACHE_SIZE 67108864UL

static PublicKey *publicKeys = NULL;

static const EVP_MD *sha512;
static const EVP_MD *ripemd160;

static int has_next_task = 1;

void *funca()
{
    PublicKey signkeys[1024];
    memcpy(signkeys, publicKeys, 1024 * PUBLIC_KEY_LENGTH);
    PublicKey enckeys[1024];
    memcpy(enckeys, publicKeys, 1024 * PUBLIC_KEY_LENGTH);
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned int mdlen = 0;

    // sign keyをローカル変数に移し替える
    // enc keyをローカル変数に移し替える
    // sign keyをハッシュコンテキストに読み込む
    // enc keyをハッシュコンテキストに読み込む
    while (has_next_task)
    {
        for (size_t sig = 0; sig < 1024; sig += 16)
        {
            for (size_t enc = 0; enc < 1024; enc += 16)
            {
                EVP_DigestInit(mdctx, sha512);
                EVP_DigestUpdate(mdctx, signkeys[sig], PUBLIC_KEY_LENGTH);
                EVP_DigestUpdate(mdctx, enckeys[enc], PUBLIC_KEY_LENGTH);
                EVP_DigestFinal(mdctx, hash, &mdlen);
                assert(mdlen == 64);
                EVP_DigestInit(mdctx, ripemd160);
                EVP_DigestUpdate(mdctx, hash, 64);
                EVP_DigestFinal(mdctx, hash, &mdlen);
            }
        }
    }
    return NULL;
}

int main(void)
{
    FILE *fin = fopen("publicKeys.bin", "rb");
    if (fin == NULL)
    {
        perror("fopen publickey");
        return EXIT_FAILURE;
    }
    publicKeys = (PublicKey *)malloc(1024 * PUBLIC_KEY_LENGTH);

    size_t keynum = fread(publicKeys, PUBLIC_KEY_LENGTH, 1024, fin);
    if (keynum < KEY_CACHE_SIZE)
    {
        perror("fread");
        free(publicKeys);
        fclose(fin);
        return EXIT_FAILURE;
    }
    fclose(fin);

    sha512 = EVP_sha512();
    ripemd160 = EVP_ripemd160();
    pthread_t t;
    pthread_create(&t, NULL, funca, NULL);
    pthread_join(t, NULL);
    free(publicKeys);
    return 0;
}
