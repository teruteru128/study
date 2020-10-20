
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
#include <omp.h>

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 67108864ULL
#define REQUIRE_NLZ 4
#define ADDRESS_VERSION 4
#define STREAM_NUMBER 1
#define J_CACHE_SIZE 126
#define BLOCK_SIZE 128

#define errchk(v, f)                                            \
  if (!v)                                                       \
  {                                                             \
    unsigned long err = ERR_get_error();                        \
    fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
    return EXIT_FAILURE;                                        \
  }

static int calcRipe(EVP_MD_CTX *mdctx, const EVP_MD *sha512, const EVP_MD *ripemd160, unsigned char *cache64, unsigned char *pubSignKey, unsigned char *pubEncKey)
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

static void validation(unsigned char *ripe, size_t i, size_t j)
{
  size_t nlz = 0;
  for (nlz = 0; !ripe[nlz] && nlz < 20; nlz++)
    ;
  if (nlz >= REQUIRE_NLZ)
  {
    //exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), signpubkey, privateKeys + (j * PRIVATE_KEY_LENGTH), encPublicKey, cache64);
#pragma omp critical
    {
      fprintf(stderr, "%ld, %ld, %ld\n", nlz, i, j);
    }
  }
}

struct searchArg
{
  unsigned char *privateKeys;
  size_t priKeyNmemb;
  unsigned char *publicKeys;
  size_t pubKeyNmemb;
  size_t signBegin;
  size_t signEnd;
  size_t encBegin;
  size_t encEnd;
  size_t minExportThreshold;
};

struct results;
struct results
{
  size_t signkeyindex;
  size_t encryptkeyindex;
  size_t numberOfLeadingZero;
  struct results *next;
};

void freeresults(struct results *results)
{
  struct results *work = NULL;
  while (results != NULL)
  {
    work = results->next;
    free(results);
    results = work->next;
  }
}

/**
 * TODO: リファクタリング
 * TODO: 鍵キャッシュサーバー
 * TODO: 既存鍵を使ってアドレス探索
 */
int main(int argc, char *argv[])
{
#ifdef _OPENMP
  printf("openmp\n");
#else
  printf("no openmp\n");
#endif
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  unsigned char *publicKeys = calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
  if (!publicKeys)
  {
    perror("calloc(publicKeys)");
    return EXIT_FAILURE;
  }
  fprintf(stderr, "calloced\n");
  {
    FILE *fin = fopen("publicKeys.bin", "rb");
    if (fin == NULL)
    {
      free(publicKeys);
      exit(EXIT_FAILURE);
    }
    size_t l = fread(publicKeys, PUBLIC_KEY_LENGTH, KEY_CACHE_SIZE, fin);
    fclose(fin);
    if (l == KEY_CACHE_SIZE)
    {
      fprintf(stderr, "ok\n");
    }
  }
  fprintf(stderr, "loaded\n");
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

  //unsigned char *privateKeys = NULL;
  //unsigned char *publicKeys = ;
  EVP_MD_CTX *mdctx = NULL;
  const EVP_MD *sha512md = EVP_sha512();
  const EVP_MD *ripemd160md = EVP_ripemd160();
  unsigned char *signpubkey = NULL;
  unsigned char *encPublicKey = NULL;
  unsigned char cache64[EVP_MAX_MD_SIZE];
  size_t iPubIndex = 0;
  size_t ii = 0;
  size_t jIndex = 0;
  size_t jj = 0;
  int r = 0;

#pragma omp parallel for shared(sha512md, ripemd160md, publicKeys) private(cache64, signpubkey, mdctx)
  for (size_t i = 0; i < 67108864; i++)
  {
    signpubkey = publicKeys + (i * PUBLIC_KEY_LENGTH);
#pragma omp parallel for shared(sha512md, ripemd160md, publicKeys) private(cache64, mdctx) firstprivate(signpubkey, i)
    for (size_t j = 0; j < i; j++)
    {
      mdctx = EVP_MD_CTX_new();
      encPublicKey = publicKeys + (j * PUBLIC_KEY_LENGTH);
      calcRipe(mdctx, sha512md, ripemd160md, cache64, signpubkey, encPublicKey);
      validation(cache64, i, j);
      calcRipe(mdctx, sha512md, ripemd160md, cache64, encPublicKey, signpubkey);
      validation(cache64, j, i);
      EVP_MD_CTX_free(mdctx);
    }
  }
  //shutdown:
  //free(privateKeys);
  free(publicKeys);
  return EXIT_SUCCESS;
}
