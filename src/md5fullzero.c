
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/md5.h>

#define THREAD_NUM 12

static size_t counter;
static pthread_mutex_t mutex;

// 極稀にしか更新しないから大丈夫、多分
static size_t j;

void *hash(void *arg)
{
  const EVP_MD *md5 = EVP_md5();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  char str[22];
  unsigned char buf[16];
  unsigned char target[16];
  size_t len = 0;
  memset(target, 0, 16);
  size_t i = 0;
  size_t i_max = 0;
  for (; j < 16;)
  {
    pthread_mutex_lock(&mutex);
    i = counter;
    counter += UINT_MAX;
    pthread_mutex_unlock(&mutex);
    i_max = i + UINT_MAX;
    for (; i < i_max; i++)
    {
      len = (size_t)snprintf(str, 22, "%zu", i);
      EVP_DigestInit(ctx, md5);
      EVP_DigestUpdate(ctx, str, len);
      EVP_DigestFinal(ctx, buf, NULL);
      // とりあえずmemcmpの呼び出し回数を256分の1に減らす
      if (buf[0])
        continue;
#if 0
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
      j++;
    }
    printf("section finished : %zu\n", i);
  }
  EVP_MD_CTX_free(ctx);
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t thread[12];
  counter = 776869784885228UL;
  j = 4;
  pthread_mutex_init(&mutex, NULL);
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_create(&thread[i], NULL, hash, &counter);
  }
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_join(thread[i], NULL);
  }
  pthread_mutex_destroy(&mutex);

  return EXIT_SUCCESS;
}
