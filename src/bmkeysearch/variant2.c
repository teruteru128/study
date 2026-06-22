
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include "queue.h"
#include <bm_sonota.h>
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
#define KEY_CACHE_SIZE 67108864UL
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
    size_t iimax;
    size_t jjmax;
    for (size_t i = 0; i < 67108864; i += BLOCK_SIZE)
    {
        iimax = i + BLOCK_SIZE;
        signingKey = signp;
        for (size_t j = 0; j < 67108864; j += BLOCK_SIZE)
        {
            encryptingKey = encp;
            jjmax = j + BLOCK_SIZE;
            for (size_t ii = 0; ii < iimax; ii++)
            {
                for (size_t jj = 0; jj < jjmax && jj <= ii; jj++)
                {
                    funcP(mdctx, signingKey, encryptingKey, hash);
                    nlz = getNLZ(hash, 20);
                    if (nlz >= 3)
                    {
                        fprintf(stderr, "%ld, %ld, %ld\n", nlz, ii, jj);
                    }
                    if (ii != jj)
                    {
                        funcP(mdctx, encryptingKey, signingKey, hash);
                        nlz = getNLZ(hash, 20);
                        if (nlz >= 3)
                        {
                            fprintf(stderr, "%ld, %ld, %ld\n", nlz, jj, ii);
                        }
                    }
                    encryptingKey += PUBLIC_KEY_LENGTH;
                }
                signingKey += PUBLIC_KEY_LENGTH;
            }
            encp += PUBLIC_KEY_LENGTH * BLOCK_SIZE;
        }
        signp += PUBLIC_KEY_LENGTH * BLOCK_SIZE;
    }
    EVP_MD_CTX_free(mdctx);
    return 0;
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) { return search_main(argc, argv); }
