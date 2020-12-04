
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <printint.h>

#ifndef THREAD_NUM
#define THREAD_NUM 12
#endif

static size_t counter;
static pthread_mutex_t mutex;

// 極稀にしか更新しないから大丈夫、多分
static size_t j;
static pthread_mutex_t mutex_j;

// 比較基準、null byteで初期化した後は書き込みをしないので共通化
static unsigned char target[16];

void *hash(void *arg)
{
  const EVP_MD *md5 = EVP_md5();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  char str[24] = "";
  unsigned char buf[16] = "";
  size_t len = 0;
  size_t i = 0;
  size_t i_max = 0;
  for (; j < 16;)
  {
    pthread_mutex_lock(&mutex);
    i = counter;
    counter += 64;
    pthread_mutex_unlock(&mutex);
    i_max = i + 64;
    for (; i < i_max; i++)
    {
      //len = (size_t)snprintf(str, 24, "%zu", i);
      len = snprintUInt64(str, 24, i);
      EVP_DigestInit(ctx, md5);
      EVP_DigestUpdate(ctx, str, len);
      EVP_DigestFinal(ctx, buf, NULL);
#if 0
      // とりあえずmemcmpの呼び出し回数を256分の1に減らす
      if (buf[0])
        continue;
      if (buf[1])
        continue;
      if (buf[2])
        continue;
      if (buf[3])
        continue;
#endif
      if (memcmp(buf, target, j))
        continue;
      fprintf(stdout, "found : %zu\n", i);
      pthread_mutex_lock(&mutex_j);
      j++;
      pthread_mutex_unlock(&mutex_j);
    }
    //printf("section finished : %zu\n", i);
  }
  EVP_MD_CTX_free(ctx);
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t thread[THREAD_NUM];

  counter = 776869784885228UL;
  j = 5;
  memset(target, 0, 16);

  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutex_j, NULL);
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_create(&thread[i], NULL, hash, &counter);
  }
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_join(thread[i], NULL);
  }
  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&mutex_j);

  return EXIT_SUCCESS;
}
