
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
#define BLOCK_SIZE 4

static const EVP_MD *sha512;
static const EVP_MD *ripemd160;

static size_t globalSignIndex = 28620;
// static pthread_mutex_t globalSignIndex_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_spinlock_t globalSignIndex_spinlock;

// 合計
static size_t globalCounts[21] = { 0 };
// static pthread_rwlock_t globalCounts_rwlock;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static PublicKey *publicKeys = NULL;
static int success = 0;
static int finished = 0;

static size_t threadNum = 1;

static size_t attempts = 0;

static void *threadFunc(void *arg)
{
    (void)arg;
    // 初期化
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    EVP_MD_CTX *sha512ctxshared[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        sha512ctxshared[i] = EVP_MD_CTX_new();
    }
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
        signIndex = globalSignIndex;
        globalSignIndex += BLOCK_SIZE;
        pthread_spin_unlock(&globalSignIndex_spinlock);
        // pthread_mutex_unlock(&globalSignIndex_mutex);
        if (signIndex >= KEY_CACHE_SIZE)
        {
            success = 1;
            finished = 1;
            // pthread_cond_broadcast(&cond);
            break;
        }

        // スレッドローカルカウンター初期化
        memset(counts, 0, sizeof(size_t) * 21);

        for (size_t i = 0; i < BLOCK_SIZE; i++)
        {
            EVP_DigestInit_ex(sha512ctxshared[i], sha512, NULL);
            EVP_DigestUpdate(sha512ctxshared[i], publicKeys[signIndex + i],
                             PUBLIC_KEY_LENGTH);
        }
        for (size_t j = 0; j < KEY_CACHE_SIZE; j++)
        {
            for (size_t i = 0; i < BLOCK_SIZE; i++)
            {
                EVP_MD_CTX_copy_ex(mdctx, sha512ctxshared[i]);
                EVP_DigestUpdate(mdctx, publicKeys[j], PUBLIC_KEY_LENGTH);
                EVP_DigestFinal_ex(mdctx, hash, &mdlen);
                assert(mdlen == 64);
                // funcP(mdctx, signingKey, encryptingKey, hash);
                EVP_DigestInit_ex(mdctx, ripemd160, NULL);
                EVP_DigestUpdate(mdctx, hash, 64);
                EVP_DigestFinal(mdctx, hash, &mdlen);
                assert(mdlen == 20);
                tmp = htobe64(*(unsigned long *)hash);
                nlz = ((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3;
                if (nlz >= 4)
                {
                    nlz = getNLZ(hash, 20);
                    // TODO:
                    // 表示をメインスレッドで行う。表示に時間を取られたくない
                    // 結果の格納領域をどういうデータ構造にするか？
                    fprintf(stdout, "%ld, %ld, %ld\n", nlz, signIndex + i, j);
                    fflush(stdout);
                }
                counts[nlz]++;
            }
        }
        // EVP_MD_CTX_reset(sha512ctxshared);

        // スレッドローカルカウンターをグローバルカウンターに適用する
        pthread_mutex_lock(&mutex);
        // pthread_rwlock_wrlock(&globalCounts_rwlock);
        for (size_t j = 0; j < 20; j++)
        {
            globalCounts[j] += counts[j];
        }
        attempts += KEY_CACHE_SIZE * BLOCK_SIZE;
        pthread_cond_broadcast(&cond);
        // pthread_rwlock_unlock(&globalCounts_rwlock);
        pthread_mutex_unlock(&mutex);
    }
    for (size_t i = 0; i < BLOCK_SIZE; i++)
        EVP_MD_CTX_free(sha512ctxshared[i]);
    EVP_MD_CTX_free(mdctx);
    return NULL;
}

static void sigint_action(int sig, siginfo_t *info, void *ctx)
{
    (void)sig;
    (void)info;
    (void)ctx;
    success = false;
    finished = true;
    // pthread_cond_broadcast をシグナルハンドラから呼び出してはいけない
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

static int search_main(int argc, char **argv)
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
    fprintf(stderr, "Number of threads : %zu\n", threadNum);
    fprintf(stderr, "Initial offset of signing key : %zu\n", globalSignIndex);

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
    struct sigaction new_action = { 0 }, old_action;
    new_action.sa_flags = SA_SIGINFO;
    new_action.sa_sigaction = sigint_action;
    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);

    sha512 = EVP_sha512();
    ripemd160 = EVP_ripemd160();

    // pthread_rwlock_init(&globalCounts_rwlock, NULL);
    pthread_spin_init(&globalSignIndex_spinlock, PTHREAD_PROCESS_SHARED);
    pthread_t *threads = malloc(threadNum * sizeof(pthread_t));
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    for (size_t i = 0; i < threadNum; i++)
    {
        if (pthread_create(threads + i, NULL, threadFunc, NULL) != 0)
        {
            perror("failed to create new thread");
            return EXIT_FAILURE;
        }
    }
    size_t a = 0;
    struct timespec now = { 0 };
    struct timespec abstime = { 0 };
    abstime.tv_sec = 1;
    struct timespec end;
    int retcode = 0;

    size_t localCounts[21] = { 0 };
    while (!finished)
    {
        pthread_mutex_lock(&mutex);
        clock_gettime(CLOCK_REALTIME, &now);
        abstime.tv_sec = now.tv_sec + 1;
        abstime.tv_nsec = now.tv_nsec;
        retcode = pthread_cond_timedwait(&cond, &mutex, &abstime);
        if (retcode != ETIMEDOUT)
        {
            for (size_t j = 0; j <= 20; j++)
            {
                localCounts[j] = globalCounts[j];
            }
            a = attempts;
        }
        pthread_mutex_unlock(&mutex);
        fputs("searched:", stderr);
        fprintf(stderr, "%lu keys", a);
        for (size_t j = 0; j < 21; j++)
        {
            fprintf(stderr, ", %zu: %zu", j, localCounts[j]);
        }
        clock_gettime(CLOCK_REALTIME, &end);
        fprintf(stderr, ", %fkeys/s\r",
                (double)a / difftime(end.tv_sec, start.tv_sec));
    }
    for (size_t i = 0; i < threadNum; i++)
    {
        pthread_join(threads[i], NULL);
    }
#ifdef DEBUG
    fputs("スレッドの終了待ち合わせが完了しました\n", stderr);
#endif
    // pthread_rwlock_destroy(&globalCounts_rwlock);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    // pthread_mutex_destroy(&globalSignIndex_mutex);
    pthread_spin_destroy(&globalSignIndex_spinlock);

    if (success)
    {
        //
    }
    else
    {
        fputs("User cancelled\n", stderr);
    }
    fprintf(stderr, "globalSignIndex : %zu\n", globalSignIndex);

    free(threads);
    free(publicKeys);
    return EXIT_SUCCESS;
}

/**
 * @brief
 *
 * @see https://github.com/CouleeApps/git-power
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) { return search_main(argc, argv); }
