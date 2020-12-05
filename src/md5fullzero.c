
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

#if 0
// 比較基準、null byteで初期化した後は書き込みをしないので共通化
static unsigned char target[16] = "";
#endif

void *hash(void *arg)
{
  const EVP_MD *md5 = EVP_md5();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  char str[24] = "";
  unsigned char buf[16] = "";
  size_t len = 0;
  size_t cnt = 0;
  size_t cnt_max = 0;
  size_t nlz;
  for (; j < 16;)
  {
    pthread_mutex_lock(&mutex);
    cnt = counter;
    counter += 1048576;
    pthread_mutex_unlock(&mutex);
    cnt_max = cnt + 1048576;
    for (; cnt < cnt_max; cnt++)
    {
      //len = (size_t)snprintf(str, 24, "%zu", cnt);
      len = snprintUInt64(str, 24, cnt);
      EVP_DigestInit(ctx, md5);
      EVP_DigestUpdate(ctx, str, len);
      EVP_DigestFinal(ctx, buf, NULL);
      if (buf[0] || buf[1] || buf[2] || buf[3])
        continue;
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
#if 0
      if (memcmp(buf, target, j))
        continue;
#endif
      for (nlz = 4; !buf[nlz] && nlz < 16; nlz++)
        ;
      if (nlz < j)
        continue;
      fprintf(stdout, "found : %zu, ", cnt);
      for (size_t k = 0; k < 16; k++)
        fprintf(stdout, "%02x", buf[k]);
      fputs("\n", stdout);
      pthread_mutex_lock(&mutex_j);
      if (nlz > j)
        j = nlz;
      pthread_mutex_unlock(&mutex_j);
    }
    //printf("section finished : %zu\n", cnt);
  }
  EVP_MD_CTX_free(ctx);
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t thread[THREAD_NUM];

  counter = 776869784885228UL;
  j = 5;

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
