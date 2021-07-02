
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include "queue.h"
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

#define KEY_CACHE_SIZE 1048576UL
#define BLOCK_SIZE 16

static const EVP_MD *sha512;
static const EVP_MD *ripemd160;

static void funcP(EVP_MD_CTX *mdctx, PublicKey *signingKey,
                  PublicKey *encryptingKey, unsigned char *hash)
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
    PublicKey *signp = publicKeys;
    PublicKey *encp = publicKeys;
    PublicKey *signingKey = publicKeys;
    PublicKey *encryptingKey = publicKeys;
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    size_t nlz = 0;
    unsigned char sum = 0;
    for (size_t i = 0; i < KEY_CACHE_SIZE;
         i += BLOCK_SIZE, signp += BLOCK_SIZE)
    {
        for (size_t j = 0; j < KEY_CACHE_SIZE;
             j += BLOCK_SIZE, encp += BLOCK_SIZE)
        {
            signingKey = signp;
            for (size_t ii = i; ii < (i + BLOCK_SIZE); ii++, signingKey++)
            {
                encryptingKey = encp;
                for (size_t jj = j; jj < (j + BLOCK_SIZE);
                     jj++, encryptingKey++)
                {
            funcP(mdctx, signingKey, encryptingKey, hash);
            for (int l = 0; l < 5; l++)
            {
                sum |= hash[l];
            }
            if (sum == 0)
            {
                nlz = sum + getNLZ(hash + 5, 15);
                if (nlz >= 5)
                {
                    fprintf(stderr, "%ld, %ld, %ld\n", nlz, ii, jj);
                }
            }
            funcP(mdctx, encryptingKey, signingKey, hash);
            sum = 0;
            for (int l = 0; l < 5; l++)
            {
                sum |= hash[l];
            }
            if (sum == 0)
            {
                nlz = sum + getNLZ(hash + 5, 15);
                if (nlz >= 5)
                {
                    fprintf(stderr, "%ld, %ld, %ld\n", nlz, jj, ii);
                }
            }
                }
            }
        }
    }
    /*
    i = 0; i < j; i++
        j = i; j < KEY_CACHE_SIZE; j++

    i = 0; i < KEY_CACHE_SIZE; i++
        j = 0; j < i; j++
    */
    for (size_t i = 0; i < KEY_CACHE_SIZE; i++, signingKey++)
    {
        encryptingKey = publicKeys;
        for (size_t j = 0; j < KEY_CACHE_SIZE; j++, encryptingKey++)
        {
            funcP(mdctx, signingKey, encryptingKey, hash);
            for (int l = 0; l < 5; l++)
            {
                sum |= hash[l];
            }
            if (sum == 0)
            {
                nlz = sum + getNLZ(hash + 5, 15);
                if (nlz >= 5)
                {
                    fprintf(stderr, "%ld, %ld, %ld\n", nlz, i, j);
                }
            }
            funcP(mdctx, encryptingKey, signingKey, hash);
            sum = 0;
            for (int l = 0; l < 5; l++)
            {
                sum |= hash[l];
            }
            if (sum == 0)
            {
                nlz = sum + getNLZ(hash + 5, 15);
                if (nlz >= 5)
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
