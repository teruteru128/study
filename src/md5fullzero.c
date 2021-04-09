
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <printint.h>
#include <nlz.h>

#ifndef THREAD_NUM
#define THREAD_NUM 12
#endif

struct globalConfig
{
  size_t counter;
  pthread_mutex_t mutex;
  size_t require_nlz;
  pthread_rwlock_t require_nlz_rwlock;
};

#if 0
// 比較基準、null byteで初期化した後は書き込みをしないので共通化
static const unsigned char target[16] = "";
#endif

void *hash(void *arg)
{
  struct globalConfig *config = (struct globalConfig *)arg;
  const EVP_MD *md5 = EVP_md5();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  char str[24] = "";
  unsigned char buf[16] = "";
  size_t len = 0;
  size_t cnt = 0;
  size_t cnt_max = 0;
  size_t nlz;
  pthread_rwlock_rdlock(&config->require_nlz_rwlock);
  for (; config->require_nlz < 16;)
  {
    pthread_rwlock_unlock(&config->require_nlz_rwlock);
    pthread_mutex_lock(&config->mutex);
    cnt = config->counter;
    config->counter += 1048576;
    pthread_mutex_unlock(&config->mutex);
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
      if (memcmp(buf, target, require_nlz))
        continue;
#endif
      nlz = getNLZ(buf, 16);
      if (nlz < config->require_nlz)
        continue;
      fprintf(stdout, "found : %zu, ", cnt);
      for (size_t k = 0; k < 16; k++)
        fprintf(stdout, "%02x", buf[k]);
      fputs("\n", stdout);
      pthread_rwlock_wrlock(&config->require_nlz_rwlock);
      if (nlz > config->require_nlz)
        config->require_nlz = nlz;
      pthread_rwlock_unlock(&config->require_nlz_rwlock);
    }
    //printf("section finished : %zu\n", cnt);
    pthread_rwlock_rdlock(&config->require_nlz_rwlock);
  }
  pthread_rwlock_unlock(&config->require_nlz_rwlock);
  EVP_MD_CTX_free(ctx);
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t thread[THREAD_NUM];

  struct globalConfig counter = {776869784885228UL, PTHREAD_MUTEX_INITIALIZER, 5, PTHREAD_RWLOCK_INITIALIZER};

  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_create(&thread[i], NULL, hash, &counter);
  }
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_join(thread[i], NULL);
  }
  pthread_mutex_destroy(&counter.mutex);
  pthread_rwlock_destroy(&counter.require_nlz_rwlock);

  return EXIT_SUCCESS;
}
