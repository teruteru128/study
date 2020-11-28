
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
#define KEY_CACHE_SIZE 67108864UL
#define J_CACHE_SIZE 126
#define LARGE_BLOCK_SIZE (KEY_CACHE_SIZE / 4)
#define SMALL_BLOCK_SIZE 64
#define REQUIRE_NLZ 4
#define THREAD_NUM 16

#define errchk(v, f)                                            \
  if (!v)                                                       \
  {                                                             \
    unsigned long err = ERR_get_error();                        \
    fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
    return EXIT_FAILURE;                                        \
  }

static int calcRipe(EVP_MD_CTX *mdctx, const EVP_MD *sha512, const EVP_MD *ripemd160, unsigned char *cache64, unsigned char *publicKey, size_t signkeyindex, size_t enckeyindex)
{
  EVP_DigestInit(mdctx, sha512);
  EVP_DigestUpdate(mdctx, publicKey + signkeyindex * PUBLIC_KEY_LENGTH, PUBLIC_KEY_LENGTH);
  EVP_DigestUpdate(mdctx, publicKey + enckeyindex * PUBLIC_KEY_LENGTH, PUBLIC_KEY_LENGTH);
  EVP_DigestFinal(mdctx, cache64, NULL);
  EVP_DigestInit(mdctx, ripemd160);
  EVP_DigestUpdate(mdctx, cache64, 64);
  EVP_DigestFinal(mdctx, cache64, NULL);
  return 0;
}

static size_t getNLZ(unsigned char *ripe)
{
  size_t nlz = 0;
  for (nlz = 0; !ripe[nlz] && nlz < 20; nlz++)
    ;
  return nlz;
}

struct threadArg
{
  unsigned char *publicKeys;
  size_t pubKeyNmemb;
  size_t signBegin;
  size_t signEnd;
  size_t encBegin;
  size_t encEnd;
  size_t minExportThreshold;
  const EVP_MD *sha512md;
  const EVP_MD *ripemd160md;
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

void *task(void *arg)
{
  struct threadArg *targ = (struct threadArg *)arg;
  unsigned char *publicKeys = targ->publicKeys;
  const EVP_MD *sha512md = targ->sha512md;
  const EVP_MD *ripemd160md = targ->ripemd160md;
  unsigned char cache64[EVP_MAX_MD_SIZE];
  size_t ii;
  size_t ii_max = targ->signEnd;
  size_t i;
  size_t i_max;
  size_t jj;
  size_t jj_max = targ->encEnd;
  size_t j;
  size_t j_max;
  size_t nlz;
  size_t minExportThreshold = targ->minExportThreshold;
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  for (ii = targ->signBegin, i_max = ii + SMALL_BLOCK_SIZE; ii < ii_max; ii = i_max, i_max += SMALL_BLOCK_SIZE)
  {
    for (jj = targ->encEnd, j_max = jj + SMALL_BLOCK_SIZE; jj < jj_max; jj = j_max, j_max += SMALL_BLOCK_SIZE)
    {
      for (i = ii; i < i_max; i++)
      {
        for (j = jj; j < j_max; j++)
        {
          calcRipe(mdctx, sha512md, ripemd160md, cache64, publicKeys, i, j);
          nlz = getNLZ(cache64);
          if (nlz >= minExportThreshold)
          {
            {
              fprintf(stderr, "%ld, %ld, %ld\n", nlz, ii, jj);
            }
          }
          calcRipe(mdctx, sha512md, ripemd160md, cache64, publicKeys, j, i);
          nlz = getNLZ(cache64);
          if (nlz >= minExportThreshold)
          {
            {
              fprintf(stderr, "%ld, %ld, %ld\n", nlz, jj, ii);
            }
          }
        }
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
  pthread_t threads[THREAD_NUM];
  struct threadArg arg[THREAD_NUM];
  const EVP_MD *sha512md = EVP_sha512();
  const EVP_MD *ripemd160md = EVP_ripemd160();
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    arg[i].publicKeys = publicKeys;
    arg[i].pubKeyNmemb = 0;
    arg[i].signBegin = LARGE_BLOCK_SIZE * (i / 4);
    arg[i].signEnd = LARGE_BLOCK_SIZE * ((i / 4) + 1);
    arg[i].encBegin = LARGE_BLOCK_SIZE * (i % 4);
    arg[i].encEnd = LARGE_BLOCK_SIZE * ((i % 4) + 1);
    arg[i].minExportThreshold = 4;
    arg[i].sha512md = sha512md;
    arg[i].ripemd160md = ripemd160md;
  }
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_create(&threads[i], NULL, task, &arg[i]);
  }
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_join(threads[i], NULL);
  }
  //shutdown:
  //free(privateKeys);
  free(publicKeys);
  return EXIT_SUCCESS;
}
