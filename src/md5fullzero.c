
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <nlz.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <printint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned char hash[16] = "";
    size_t len = 0;
    size_t cnt = 0;
    size_t cnt_max = 0;
    size_t nlz;
    pthread_rwlock_rdlock(&config->require_nlz_rwlock);
    unsigned int sum = 0;
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
            len = (size_t)snprintf(str, 24, "%zu", cnt);
            // len = snprintUInt64(str, 24, cnt);
            EVP_DigestInit(ctx, md5);
            EVP_DigestUpdate(ctx, str, len);
            EVP_DigestFinal(ctx, hash, NULL);
            sum = 0;
            for (int i = 0; i < 4; i++)
                sum |= hash[i];
            if (sum != 0)
                continue;
#if 0
      // とりあえずmemcmpの呼び出し回数を256分の1に減らす
      if (hash[0])
        continue;
      if (hash[1])
        continue;
      if (hash[2])
        continue;
      if (hash[3])
        continue;
#endif
#if 0
      if (memcmp(hash, target, require_nlz))
        continue;
#endif
            nlz = getNLZ(hash, 16);
            if (nlz < config->require_nlz)
                continue;
            fprintf(stdout, "found : %zu, ", cnt);
            for (size_t k = 0; k < 16; k++)
                fprintf(stdout, "%02x", hash[k]);
            fputs("\n", stdout);
            pthread_rwlock_wrlock(&config->require_nlz_rwlock);
            if (nlz > config->require_nlz)
                config->require_nlz = nlz;
            pthread_rwlock_unlock(&config->require_nlz_rwlock);
        }
        // printf("section finished : %zu\n", cnt);
        pthread_rwlock_rdlock(&config->require_nlz_rwlock);
    }
    pthread_rwlock_unlock(&config->require_nlz_rwlock);
    EVP_MD_CTX_free(ctx);
    return NULL;
}

static struct globalConfig *globalConfig_new(size_t counter, size_t nlz)
{
    struct globalConfig *config = malloc(sizeof(struct globalConfig));
    config->counter = counter;
    pthread_mutex_init(&config->mutex, NULL);
    config->require_nlz = nlz;
    pthread_rwlock_init(&config->require_nlz_rwlock, NULL);
    return config;
}

static void globalConfig_destory(struct globalConfig *config)
{
    config->counter = 0;
    pthread_mutex_destroy(&config->mutex);
    config->require_nlz = 0;
    pthread_rwlock_destroy(&config->require_nlz_rwlock);
}

int main(int argc, char **argv)
{
    size_t threadNum = THREAD_NUM;
    if (argc > 2)
    {
        threadNum = (size_t)strtoul(argv[1], NULL, 0);
    }
    pthread_t *threads = malloc(threadNum * sizeof(pthread_t));

    struct globalConfig *config = globalConfig_new(776869784885228UL, 5);

    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&threads[i], NULL, hash, &config);
    }
    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    globalConfig_destory(config);
    free(config);
    free(threads);

    return EXIT_SUCCESS;
}
