
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

static size_t globalSignIndex = 81213;
// static pthread_mutex_t globalSignIndex_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_spinlock_t globalSignIndex_spinlock;

// 合計
size_t globalCounts[21] = { 0 };
// static pthread_rwlock_t globalCounts_rwlock;
static pthread_mutex_t globalCounts_mutex = PTHREAD_MUTEX_INITIALIZER;

static PublicKey *publicKeys = NULL;
static int success = 0;
static int finished = 0;

static size_t threadNum = 15;

void *threadFunc(void *arg)
{
    (void)arg;
    // 初期化
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    EVP_MD_CTX *sha512ctxshared = EVP_MD_CTX_new();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    size_t nlz = 0;
    unsigned int mdlen = 0;
    unsigned long tmp = 0;
    // 小計
    size_t counts[21] = { 0 };

    size_t signIndex = 0;
    while (!finished)
    {

        // pthread_mutex_lock(&globalSignIndex_mutex);
        pthread_spin_lock(&globalSignIndex_spinlock);
        signIndex = globalSignIndex++;
        pthread_spin_unlock(&globalSignIndex_spinlock);
        // pthread_mutex_unlock(&globalSignIndex_mutex);
        if (signIndex >= KEY_CACHE_SIZE)
        {
            EVP_MD_CTX_free(sha512ctxshared);
            EVP_MD_CTX_free(mdctx);
            finished = 1;
            return NULL;
        }

        // スレッドローカルカウンター初期化
        for (size_t j = 0; j < 21; j++)
        {
            counts[j] = 0;
        }

        EVP_DigestInit_ex(sha512ctxshared, sha512, NULL);
        EVP_DigestUpdate(sha512ctxshared, publicKeys[signIndex],
                         PUBLIC_KEY_LENGTH);
        for (size_t j = 0; j < KEY_CACHE_SIZE; j++)
        {
            // EVP_DigestInit_ex(mdctx, sha512, NULL);
            EVP_MD_CTX_copy_ex(mdctx, sha512ctxshared);
            EVP_DigestUpdate(mdctx, publicKeys[j], PUBLIC_KEY_LENGTH);
            EVP_DigestFinal_ex(mdctx, hash, &mdlen);
            assert(mdlen == 64);
            // funcP(mdctx, signingKey, encryptingKey, hash);
            EVP_DigestInit_ex(mdctx, ripemd160, NULL);
            EVP_DigestUpdate(mdctx, hash, 64);
            EVP_DigestFinal(mdctx, hash, &mdlen);
            assert(mdlen == 20);
            tmp = htole64(*(unsigned long *)hash);
            nlz = ((tmp == 0) ? 64UL : (size_t)__builtin_ctzl(tmp)) >> 3;
            if (nlz >= 4)
            {
                nlz = getNLZ(hash, 20);
                // TODO: 表示をメインスレッドで行う。表示に時間を取られたくない
                fprintf(stdout, "%ld, %ld, %ld\n", nlz, signIndex, j);
                fflush(stdout);
            }
            counts[nlz]++;
        }
        // EVP_MD_CTX_reset(sha512ctxshared);

        // スレッドローカルカウンターをグローバルカウンターに適用する
        pthread_mutex_lock(&globalCounts_mutex);
        // pthread_rwlock_wrlock(&globalCounts_rwlock);
        for (size_t j = 0; j <= 20; j++)
        {
            globalCounts[j] += counts[j];
        }
        // TODO: 表示をメインスレッドで行う。表示に時間を取られたくない
        for (size_t j = 0; j < 21; j++)
        {
            if (j != 0)
                fputs(", ", stderr);
            fprintf(stderr, "%zu : %zu", j, globalCounts[j]);
        }
        fputs("\n", stderr);
        // pthread_rwlock_unlock(&globalCounts_rwlock);
        pthread_mutex_unlock(&globalCounts_mutex);
    }
    EVP_MD_CTX_free(sha512ctxshared);
    EVP_MD_CTX_free(mdctx);
    return NULL;
}

static void sigint_action(int sig)
{
    (void)sig;
    fputs("終了しています。お待ち下さい。。。\n", stderr);
    success = false;
    finished = true;
}

static int isHelpMode = 0;

static void parseArgs(int argc, char **argv)
{
    char *catch = NULL;
    unsigned long tmp = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            isHelpMode = 1;
        }
        else if ((strcmp(argv[i], "--thread") == 0
                  || strcmp(argv[i], "-t") == 0)
                 && (i + 1) < argc)
        {
            tmp = strtoul(argv[i + 1], &catch, 10);
            if (catch != argv[i + i])
            {
                if (tmp == 0)
                {
                    long availableProcessors = sysconf(_SC_NPROCESSORS_ONLN);
                    if (availableProcessors >= 0)
                    {
                        threadNum = (size_t)availableProcessors;
                    }
                }
                else if (tmp > 0)
                {
                    threadNum = tmp;
                }
            }
            i++;
        }
        else if ((strcmp(argv[i], "--offset") == 0
                  || strcmp(argv[i], "-o") == 0)
                 && (i + 1) < argc)
        {
            tmp = (size_t)strtoul(argv[i + 1], &catch, 10);
            if (catch != argv[i + i])
                globalSignIndex = tmp;
            i++;
        }
    }
}

int search_main(int argc, char **argv)
{
    parseArgs(argc, argv);
    if (isHelpMode != 0)
    {
        fprintf(stderr, "%s <options>\n", argv[0]);
        fprintf(stderr, "--help\n");
        fprintf(stderr, "--thread -t: スレッド数\n");
        // 私はオフセットには「-o」オプションを割り当てるべきでないと思います。「-f」オプションを割り当てるべきでしょうか？
        fprintf(stderr, "--offset -o: オフセット\n");
        return EXIT_FAILURE;
    }

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
    struct sigaction a = { 0 };
    a.sa_handler = sigint_action;
    if (sigaction(SIGINT, &a, NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        free(publicKeys);
        return EXIT_FAILURE;
    }

    sha512 = EVP_sha512();
    ripemd160 = EVP_ripemd160();
    fprintf(stderr, "Number of threads:%zu\n", threadNum);
    fprintf(stderr, "Initial offset of signing key:%zu\n", globalSignIndex);

    // pthread_rwlock_init(&globalCounts_rwlock, NULL);
    pthread_spin_init(&globalSignIndex_spinlock, PTHREAD_PROCESS_SHARED);
    pthread_t *threads = malloc(threadNum * sizeof(pthread_t));
    for (size_t i = 0; i < threadNum; i++)
    {
        if (pthread_create(threads + i, NULL, threadFunc, NULL) != 0)
        {
            perror("failed to create new thread");
            return EXIT_FAILURE;
        }
    }
    for (size_t i = 0; i < threadNum; i++)
    {
        pthread_join(threads[i], NULL);
    }
#ifdef DEBUG
    fputs("スレッドの終了待ち合わせが完了しました\n", stderr);
#endif
    // pthread_rwlock_destroy(&globalCounts_rwlock);
    pthread_mutex_destroy(&globalCounts_mutex);
    // pthread_mutex_destroy(&globalSignIndex_mutex);
    pthread_spin_destroy(&globalSignIndex_spinlock);
    if (!success)
    {
        fputs("User cancelled\n", stderr);
    }
    fprintf(stderr, "globalSignIndex : %zu\n", globalSignIndex);

    free(threads);
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
