
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
//#include "queue.h"
#include <assert.h>
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
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define KEY_CACHE_SIZE 67108864UL
#define BLOCK_SIZE 16

static const EVP_MD *sha512;
static const EVP_MD *ripemd160;

static size_t globalSignIndex = 1557;
// static pthread_mutex_t globalSignIndex_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_spinlock_t gloablSignIndex_spinlock;

// 合計
size_t globalCounts[21] = { 0 };
static pthread_rwlock_t globalCounts_rwlock;

static PublicKey *publicKeys = NULL;
static int success = 0;
static int finished = 0;

#define THREAD_NUM 15

void *threadFunc(void *arg)
{
    (void)arg;
    // 初期化
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    EVP_MD_CTX *sha512ctxshared = EVP_MD_CTX_new();
    EVP_MD_CTX *sha512ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(sha512ctx, sha512, NULL);
    EVP_MD_CTX *ripemdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ripemdctx, ripemd160, NULL);
    size_t nlz = 0;
    unsigned int mdlen = 0;
    unsigned long tmp = 0;
    // 小計
    size_t counts[21] = { 0 };

    size_t signIndex = 0;
    for (;;)
    {
        if (finished)
            return NULL;

        // pthread_mutex_lock(&signIndex_mutex);
        pthread_spin_lock(&gloablSignIndex_spinlock);
        signIndex = globalSignIndex++;
        pthread_spin_unlock(&gloablSignIndex_spinlock);
        // pthread_mutex_unlock(&signIndex_mutex);
        if (signIndex >= KEY_CACHE_SIZE)
        {
            finished = 1;
            return NULL;
        }

        // スレッドローカルカウンター初期化
        for (size_t j = 0; j <= 20; j++)
        {
            counts[j] = 0;
        }

        EVP_DigestInit_ex(sha512ctxshared, sha512, NULL);
        EVP_DigestUpdate(sha512ctxshared, publicKeys[signIndex],
                         PUBLIC_KEY_LENGTH);
        for (size_t j = 0; j < KEY_CACHE_SIZE; j++)
        {
            // EVP_DigestInit_ex(sha512ctx, sha512, NULL);
            EVP_MD_CTX_copy_ex(sha512ctx, sha512ctxshared);
            EVP_DigestUpdate(sha512ctx, publicKeys[j], PUBLIC_KEY_LENGTH);
            EVP_DigestFinal_ex(sha512ctx, hash, &mdlen);
            assert(mdlen == 64);
            // funcP(sha512ctx, signingKey, encryptingKey, hash);
            EVP_DigestInit_ex(ripemdctx, ripemd160, NULL);
            EVP_DigestUpdate(ripemdctx, hash, 64);
            EVP_DigestFinal(ripemdctx, hash, &mdlen);
            assert(mdlen == 20);
            tmp = htole64(*(unsigned long *)hash);
            nlz = ((tmp == 0) ? 64 : (unsigned int)__builtin_ctzl(tmp)) >> 3;
            if (nlz >= 4)
            {
                nlz = getNLZ(hash, 20);
                fprintf(stdout, "%ld, %ld, %ld\n", nlz, signIndex, j);
            }
            counts[nlz]++;
        }
        // EVP_MD_CTX_reset(sha512ctxshared);

        // スレッドローカルカウンターをグローバルカウンターに適用する
        pthread_rwlock_wrlock(&globalCounts_rwlock);
        for (size_t j = 0; j <= 20; j++)
        {
            globalCounts[j] += counts[j];
        }
        for (size_t j = 0; j < 21; j++)
        {
            if (j != 0)
                fputs(", ", stderr);
            fprintf(stderr, "%zu : %zu", j, globalCounts[j]);
        }
        fputs("\n", stderr);
        pthread_rwlock_unlock(&globalCounts_rwlock);
    }
    EVP_MD_CTX_free(sha512ctxshared);
    EVP_MD_CTX_free(sha512ctx);
    return NULL;
}

static void sigint_action(int sig)
{
    (void)sig;
    fputs("終了しています。お待ち下さい。。。\n", stderr);
    success = false;
    finished = true;
}

int search_main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    FILE *fin = fopen("publicKeys.bin", "rb");
    if (fin == NULL)
    {
        perror("fopen publickey");
        return EXIT_FAILURE;
    }
    publicKeys = (PublicKey *)malloc(KEY_CACHE_SIZE * PUBLIC_KEY_LENGTH);

    size_t keynum = fread(publicKeys, PUBLIC_KEY_LENGTH, KEY_CACHE_SIZE, fin);

    if (keynum < KEY_CACHE_SIZE)
    {
        perror("fread");
        free(publicKeys);
        fclose(fin);
        return EXIT_FAILURE;
    }
    fclose(fin);
    signal(SIGINT, &sigint_action);

    sha512 = EVP_sha512();
    ripemd160 = EVP_ripemd160();

    pthread_rwlock_init(&globalCounts_rwlock, NULL);
    pthread_spin_init(&gloablSignIndex_spinlock, PTHREAD_PROCESS_SHARED);
    pthread_t threads[THREAD_NUM];
    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        if (pthread_create(threads + i, NULL, threadFunc, NULL) != 0)
        {
            perror("failed to create new thread");
            return EXIT_FAILURE;
        }
    }
    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_rwlock_destroy(&globalCounts_rwlock);
    // pthread_mutex_destroy(&globalSignIndex_mutex);
    pthread_spin_destroy(&gloablSignIndex_spinlock);
    fprintf(stderr, "globalSignIndex : %zu\n", globalSignIndex);

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
