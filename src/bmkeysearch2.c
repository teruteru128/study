
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <random.h>
#include <bm.h>
#include "queue.h"
#include <nlz.h>

#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 67108864UL

int search_main(int argc, char **argv)
{
  FILE *fin = fopen("publicKeys.bin", "rb");
  if (fin == NULL)
  {
    perror("fopen publickey");
    return EXIT_FAILURE;
  }
  char *publicKeys = calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
  size_t keynum = fread(publicKeys, PUBLIC_KEY_LENGTH, KEY_CACHE_SIZE, fin);
  if (keynum < KEY_CACHE_SIZE)
  {
    perror("fread");
    free(publicKeys);
    fclose(fin);
  }
  const EVP_MD *sha512 = EVP_sha512();
  const EVP_MD *ripemd160 = EVP_ripemd160();
  char *signingKey = publicKeys;
  char *encryptingKey = publicKeys;
  unsigned char hash[EVP_MAX_MD_SIZE] = "";
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  size_t nlz = 0;
  for (size_t i = 0; i < 16; i++)
  {
    for (size_t j = 0; j <= i; j++)
    {
      EVP_DigestInit(mdctx, sha512);
      EVP_DigestUpdate(mdctx, signingKey, PUBLIC_KEY_LENGTH);
      EVP_DigestUpdate(mdctx, encryptingKey, PUBLIC_KEY_LENGTH);
      EVP_DigestFinal(mdctx, hash, NULL);
      EVP_DigestInit(mdctx, ripemd160);
      EVP_DigestUpdate(mdctx, hash, 64);
      EVP_DigestFinal(mdctx, hash, NULL);
      nlz = getNLZ(hash, 20);
      if (nlz >= 5)
      {
        {
          fprintf(stderr, "%ld, %ld, %ld\n", nlz, i, j);
        }
      }
      if (i != j)
      {
        EVP_DigestInit(mdctx, sha512);
        EVP_DigestUpdate(mdctx, encryptingKey, PUBLIC_KEY_LENGTH);
        EVP_DigestUpdate(mdctx, signingKey, PUBLIC_KEY_LENGTH);
        EVP_DigestFinal(mdctx, hash, NULL);
        EVP_DigestInit(mdctx, ripemd160);
        EVP_DigestUpdate(mdctx, hash, 64);
        EVP_DigestFinal(mdctx, hash, NULL);
        nlz = getNLZ(hash, 20);
        if (nlz >= 5)
        {
          {
            fprintf(stderr, "%ld, %ld, %ld\n", nlz, j, i);
          }
        }
      }
      encryptingKey += PUBLIC_KEY_LENGTH;
    }
    encryptingKey = publicKeys;
    signingKey += PUBLIC_KEY_LENGTH;
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
int main(int argc, char *argv[])
{
  return search_main(argc, argv);
}
