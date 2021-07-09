
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include "queue.h"
#include <assert.h>
#include <bitmessage.h>
#include <bm.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <nlz.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <random.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define KEY_CACHE_SIZE 67108864UL
#define BLOCK_SIZE 16

static const EVP_MD *sha512;
static const EVP_MD *ripemd160;

int search_main(int argc, char **argv)
{
    FILE *fin = fopen("publicKeys.bin", "rb");
    if (fin == NULL)
    {
        perror("fopen publickey");
        return EXIT_FAILURE;
    }
    PublicKey *publicKeys
        = (PublicKey *)calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
    size_t keynum = fread(publicKeys, PUBLIC_KEY_LENGTH, KEY_CACHE_SIZE, fin);
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
    // i と countsをグローバルにして読み書きのときだけロックすればいいか？？？？
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    EVP_MD_CTX *sha512ctxshared = EVP_MD_CTX_new();
    EVP_MD_CTX *sha512ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(sha512ctx, sha512, NULL);
    EVP_MD_CTX *ripemdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ripemdctx, ripemd160, NULL);
    size_t nlz = 0;
    unsigned int mdlen = 0;
    unsigned long tmp = 0;
    size_t counts[21] = { 0 };

    /*
    i = 0; i < j; i++
        j = i; j < KEY_CACHE_SIZE; j++

    i = 0; i < KEY_CACHE_SIZE; i++
        j = 0; j < i; j++
    */
    unsigned char sum = 0;
    for (size_t i = 0; i < KEY_CACHE_SIZE; i++)
    {
        EVP_DigestInit_ex(sha512ctxshared, sha512, NULL);
        EVP_DigestUpdate(sha512ctxshared, publicKeys[i], PUBLIC_KEY_LENGTH);
        for (size_t j = 0; j < KEY_CACHE_SIZE; j++)
        {
            // EVP_DigestInit_ex(sha512ctx, sha512, NULL);
            EVP_MD_CTX_copy_ex(sha512ctx, sha512ctxshared);
            EVP_DigestUpdate(sha512ctx, publicKeys[j], PUBLIC_KEY_LENGTH);
            EVP_DigestFinal_ex(sha512ctx, hash, &mdlen);
            assert(mdlen == 64);
            // funcP(sha512ctx, signingKey, encryptingKey, hash);
            EVP_DigestInit_ex(ripemdctx, ripemd160, NULL);
            EVP_DigestUpdate(ripemdctx, hash, 64);
            EVP_DigestFinal(ripemdctx, hash, &mdlen);
            assert(mdlen == 20);
            tmp = htobe64(*(unsigned long *)hash);
            nlz = ((tmp == 0) ? 64 : (unsigned int)__builtin_ctzl(tmp)) >> 3;
            if (nlz >= 5)
            {
                nlz = sum + getNLZ(hash, 20);
                if (nlz >= 5)
                {
                    fprintf(stderr, "%ld, %ld, %ld\n", nlz, i, j);
                }
            }
            counts[nlz]++;
        }
        // EVP_MD_CTX_reset(sha512ctxshared);
        for (size_t j = 0; j <= 20; j++)
        {
            if (j != 0)
                fputs(", ", stdout);
            printf("%zu : %16zu", j, counts[j]);
        }
        fputs("\n", stdout);
    }
    EVP_MD_CTX_free(sha512ctxshared);
    EVP_MD_CTX_free(sha512ctx);
    free(publicKeys);
    return EXIT_FAILURE;
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) { return search_main(argc, argv); }
