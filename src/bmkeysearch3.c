
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include "queue.h"
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

#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 1048576UL
#define BLOCK_SIZE 256

static const EVP_MD *sha512;
static const EVP_MD *ripemd160;

static void funcP(EVP_MD_CTX *mdctx, unsigned char *signingKey,
                  unsigned char *encryptingKey, unsigned char *hash)
{
    EVP_DigestInit(mdctx, sha512);
    EVP_DigestUpdate(mdctx, signingKey, PUBLIC_KEY_LENGTH);
    EVP_DigestUpdate(mdctx, encryptingKey, PUBLIC_KEY_LENGTH);
    EVP_DigestFinal(mdctx, hash, NULL);
    EVP_DigestInit(mdctx, ripemd160);
    EVP_DigestUpdate(mdctx, hash, 64);
    EVP_DigestFinal(mdctx, hash, NULL);
}

int search_main(int argc, char **argv)
{
    FILE *fin = fopen("publicKeys.bin", "rb");
    if (fin == NULL)
    {
        perror("fopen publickey");
        return EXIT_FAILURE;
    }
    unsigned char *publicKeys = calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
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
    unsigned char *signp = publicKeys;
    unsigned char *encp = publicKeys;
    unsigned char *signingKey = publicKeys;
    unsigned char *encryptingKey = publicKeys;
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    size_t nlz = 0;
    /*
    i = 0; i < j; i++
        j = i; j < KEY_CACHE_SIZE; j++

    i = 0; i < KEY_CACHE_SIZE; i++
        j = 0; j < i; j++
    */
    unsigned char sum = 0;
    for (size_t i = 0; i < KEY_CACHE_SIZE; i++, signingKey += PUBLIC_KEY_LENGTH)
    {
        // /encryptingKey = publicKeys;
        for (size_t j = 0; j < i; j++, encryptingKey += PUBLIC_KEY_LENGTH)
        {
            funcP(mdctx, signingKey, encryptingKey, hash);
            sum = 0;
            for (int l = 0; l < 5; l++)
            {
                sum |= hash[i];
            }
            if (sum == 0)
            {
                nlz = sum + getNLZ(hash + 5, 15);
                if (nlz >= 2)
                {
                    fprintf(stderr, "%ld, %ld, %ld\n", nlz, i, j);
                }
            }
            funcP(mdctx, encryptingKey, signingKey, hash);
            sum = 0;
            for (int l = 0; l < 5; l++)
            {
                sum |= hash[i];
            }
            if (sum == 0)
            {
                nlz = sum + getNLZ(hash + 5, 15);
                if (nlz >= 2)
                {
                    fprintf(stderr, "%ld, %ld, %ld\n", nlz, j, i);
                }
            }
        }
    }
    EVP_MD_CTX_free(mdctx);
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
