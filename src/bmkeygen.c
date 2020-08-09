
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <random.h>
#include <bm.h>
#include <pthread.h>

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 67108864ULL
#define REQUIRE_NLZ 4
#define ADDRESS_VERSION 4
#define STREAM_NUMBER 1
#define J_CACHE_SIZE 126
#define BLOCK_SIZE 128

#define errchk(v, f)                                                \
    if (!v)                                                         \
    {                                                               \
        unsigned long err = ERR_get_error();                        \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
        return EXIT_FAILURE;                                        \
    }

static int calcRipe(EVP_MD_CTX *mdctx, const EVP_MD *sha512, const EVP_MD *ripemd160, char *cache64, char *pubSignKey, char *pubEncKey)
{
    EVP_DigestInit(mdctx, sha512);
    EVP_DigestUpdate(mdctx, pubSignKey, PUBLIC_KEY_LENGTH);
    EVP_DigestUpdate(mdctx, pubEncKey, PUBLIC_KEY_LENGTH);
    EVP_DigestFinal(mdctx, cache64, NULL);
    EVP_DigestInit(mdctx, ripemd160);
    EVP_DigestUpdate(mdctx, cache64, 64);
    EVP_DigestFinal(mdctx, cache64, NULL);
    return 0;
}

struct searchArg
{
    unsigned char *privateKeys;
    size_t priKeyNmemb;
    unsigned char *publicKeys;
    size_t pubKeyNmemb;
    size_t ibegin;
    size_t iend;
    size_t jbegin;
    size_t jend;
    size_t minExportThreshold;
};

void *searchAddress(void *arg)
{
    struct searchArg *searchArg = (struct searchArg *)arg;
    if (!searchAddress)
        return NULL;

    unsigned char *privateKeys = searchArg->privateKeys;
    unsigned char *publicKeys = searchArg->publicKeys;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *sha512md = EVP_sha512();
    const EVP_MD *ripemd160md = EVP_ripemd160();
    unsigned char *iPublicKey;
    unsigned char *jPublicKey;
    unsigned char cache64[EVP_MAX_MD_SIZE];
    size_t i = 0;
    size_t iPubIndex = 0;
    size_t ii = 0;
    size_t j = 0;
    size_t jIndex = 0;
    size_t jj = 0;
    int r = 0;
    size_t nlz = 0;

    for (i = 0; i < KEY_CACHE_SIZE; i++)
    {
        iPubIndex = i * PUBLIC_KEY_LENGTH;
        iPublicKey = publicKeys + iPubIndex;
        // ヒープから直接参照するより一度スタックにコピーしたほうが早い説
        calcRipe(mdctx, sha512md, ripemd160md, cache64, iPublicKey, iPublicKey);
        if (!cache64[0])
        {
            for (nlz = 1; !cache64[nlz] && nlz < 20; nlz++)
                ;
            if (nlz >= REQUIRE_NLZ)
                exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
        }
        for (j = 0; j < i; j++)
        {
            jPublicKey = publicKeys + (j * PUBLIC_KEY_LENGTH);
            calcRipe(mdctx, sha512md, ripemd160md, cache64, iPublicKey, jPublicKey);
            if (!cache64[0])
            {
                for (nlz = 1; !cache64[nlz] && nlz < 20; nlz++)
                    ;
                if (nlz >= REQUIRE_NLZ)
                    exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + (j * PRIVATE_KEY_LENGTH), jPublicKey, cache64);
            }
            calcRipe(mdctx, sha512md, ripemd160md, cache64, jPublicKey, iPublicKey);
            if (!cache64[0])
            {
                for (nlz = 1; !cache64[nlz] && nlz < 20; nlz++)
                    ;
                if (nlz >= REQUIRE_NLZ)
                    exportAddress(privateKeys + (j * PRIVATE_KEY_LENGTH), jPublicKey, privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
            }
        }
    }
    EVP_MD_CTX_free(mdctx);
    return NULL;
}

/**
 * TODO: リファクタリング
 * TODO: 鍵キャッシュサーバー
 * TODO: 既存鍵を使ってアドレス探索
 */
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    unsigned char *publicKeys = calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
    if (!publicKeys)
    {
        perror("calloc(publicKeys)");
        return EXIT_FAILURE;
    }
    {
        FILE *fin = fopen("publicKeys.bin", "rb");
        if (fin == NULL)
        {
            free(publicKeys);
            exit(EXIT_FAILURE);
        }
        size_t l = fread(publicKeys, PUBLIC_KEY_LENGTH, KEY_CACHE_SIZE, fin);
        fclose(fin);
    }
    pthread_t pthreads[4];
    /*
     * 67108864
     * 67108864(67108864+1)/2 = 2251799847239680
     * x(x+1)/2 = 562949961809920
     * x≈33554432
     * x≈47453132
     * x≈58117980
     * threads[0]:0<=x<33554432
     * threads[1]:33554432<=x<47453132
     * threads[2]:47453132<=x<58117980
     * threads[3]:58117980<=x<67108864
     */
    size_t jbegins[4] = {0, 33554432, 47453132, 58117980};
    size_t jends[4] = {33554432, 47453132, 58117980, 67108864};
    struct searchArg arg[4];
    for (size_t i = 0; i < 4; i++)
    {
        arg[i].privateKeys = NULL;
        arg[i].priKeyNmemb = 0;
        arg[i].publicKeys = publicKeys;
        arg[i].pubKeyNmemb = 67108864;
        arg[i].ibegin = 0;
        arg[i].iend = 67108864;
        arg[i].jbegin = jbegins[i];
        arg[i].jend = jends[i];
    }
    /* DEAD CODE ***********************/
    //shutdown:
    //free(privateKeys);
    free(publicKeys);
    return EXIT_SUCCESS;
}
