
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
#include <bm_sonota.h>
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
#include <stdatomic.h>
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

static atomic_size_t globalSignIndex = 0;

// 合計
static atomic_size_t globalCounts[21] = { 0 };
// static pthread_rwlock_t globalCounts_rwlock;
static pthread_mutex_t globalCounts_mutex = PTHREAD_MUTEX_INITIALIZER;

static PublicKey *publicKeys = NULL;
static volatile sig_atomic_t success = 0;
static volatile sig_atomic_t finished = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static size_t threadNum = 15;

static void *threadFunc(void *arg)
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

        signIndex = globalSignIndex++;
        if (signIndex >= KEY_CACHE_SIZE)
        {
            success = 1;
            finished = 1;
            // pthread_cond_broadcast(&cond);
            break;
        }

        // スレッドローカルカウンター初期化
        memset(counts, 0, sizeof(size_t) * 21);

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
            tmp = htobe64(*(unsigned long *)hash);
            nlz = ((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3;
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
        for (size_t j = 0; j < 20; j++)
        {
            globalCounts[j] += counts[j];
        }
        // TODO: 表示をメインスレッドで行う。表示に時間を取られたくない
        for (size_t j = 0; j <= 20; j++)
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
    // fputs をシグナルハンドラから呼び出してはいけない https://www.jpcert.or.jp/sc-rules/c-sig30-c.html
    // fputs("終了しています。お待ち下さい。。。\n", stderr);
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
    struct sigaction action = { 0 };
    action.sa_handler = sigint_action;
    if (sigaction(SIGINT, &action, NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        free(publicKeys);
        return EXIT_FAILURE;
    }

    sha512 = EVP_sha512();
    ripemd160 = EVP_ripemd160();

    pthread_t *threads = malloc(threadNum * sizeof(pthread_t));
    for (size_t i = 0; i < threadNum; i++)
    {
        if (pthread_create(threads + i, NULL, threadFunc, NULL) != 0)
        {
            perror("failed to create new thread");
            return EXIT_FAILURE;
        }
    }
    /*
    struct timespec spec = { 0 };
    spec.tv_sec = 1;
    while (!finished)
    {
        pthread_mutex_lock(&mutex);
        pthread_cond_timedwait(&cond, &mutex, &spec);
        pthread_mutex_unlock(&mutex);
    }
     */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGINT);
    siginfo_t info = { 0 };
    struct timespec timeout = { 0 };
    // 0.5 s
    timeout.tv_sec = 0;
    timeout.tv_nsec = 500000000;
    while (!finished)
    {
        sigtimedwait(&set, &info, &timeout);
    }
    fputs("終了しています。お待ち下さい。。。\n", stdout);
    for (size_t i = 0; i < threadNum; i++)
    {
        pthread_join(threads[i], NULL);
    }
    fputs("スレッドの終了待ち合わせが完了しました\n", stderr);
    // pthread_rwlock_destroy(&globalCounts_rwlock);
    pthread_mutex_destroy(&globalCounts_mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

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
    return EXIT_FAILURE;
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
