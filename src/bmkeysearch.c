
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include "queue.h"
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
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "bmkeyload.h"

#define KEY_CACHE_SIZE 67108864UL
#define J_CACHE_SIZE 126
#define LARGE_BLOCK_SIZE (KEY_CACHE_SIZE / 4)
#define SMALL_BLOCK_SIZE 64
#define REQUIRE_NLZ 4
#define THREAD_NUM 12
#define TASK_SIZE 16

static PublicKey *publicKeys = NULL;
static const EVP_MD *sha512md = NULL;
static const EVP_MD *ripemd160md = NULL;

/**
 * @brief threadpoolを停止するときは0を代入する
 */
static volatile int threadpool_live = 1;
static volatile int producer_has_a_task_that_has_not_been_shipped = 1;

#define errchk(v, f)                                                          \
    if (!v)                                                                   \
    {                                                                         \
        unsigned long err = ERR_get_error();                                  \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL));           \
        return EXIT_FAILURE;                                                  \
    }

struct threadArg
{
    PublicKey *publicKeys;
    size_t pubKeyNmemb;
    size_t signBegin;
    size_t signEnd;
    size_t encBegin;
    size_t encEnd;
    size_t minExportThreshold;
    struct queue *queue;
};

struct task;
struct task
{
    size_t signkeyindex;
    size_t encryptkeyindex;
    size_t numberOfLeadingZero;
};

void *produce(void *arg)
{
    struct queue *queue = (struct queue *)arg;

    for (size_t j = 0; j < 100000 * TASK_SIZE; j += TASK_SIZE)
    {
        struct task *task = calloc(1, sizeof(struct task));
        task->signkeyindex = 0;
        task->encryptkeyindex = j;
        task->numberOfLeadingZero = 0;
        put(queue, queue);
    }

    /*
    for (size_t i = 0; i < KEY_CACHE_SIZE; i += TASK_SIZE)
    {
        for (size_t j = 0; j < KEY_CACHE_SIZE; j += TASK_SIZE)
        {
            struct task *task = calloc(1, sizeof(struct task));
            task->signkeyindex = i;
            task->encryptkeyindex = j;
            task->numberOfLeadingZero = 0;
        }
    }
    */
    return NULL;
}

struct task *getTask(struct queue *queue) { return take(queue); }

void calcsmallTask(PublicKey *pubkeys, size_t a, size_t b, EVP_MD_CTX *ctx)
{
    return;
}

void *consume(void *arg)
{
    struct threadArg *targ = (struct threadArg *)arg;
    unsigned char cache64[EVP_MAX_MD_SIZE];
    size_t ii;
    size_t ii_max = targ->signEnd;
    size_t signIndex;
    size_t signIndexMax;
    size_t i_max;
    size_t jj;
    size_t jj_max = targ->encEnd;
    size_t encIndex;
    size_t encIndexMax;
    size_t j_max;
    size_t nlz;
    size_t minExportThreshold = targ->minExportThreshold;
    size_t maxNLZ = 0;
    struct queue *queue = targ->queue;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    PublicKey *signP = NULL;
    PublicKey *encP = NULL;
    while (1)
    {
        struct task *task = getTask(queue);
        if (task == NULL)
        {
            continue;
        }
        signIndex = task->signkeyindex;
        signIndexMax = signIndex + TASK_SIZE;
        encIndex = task->encryptkeyindex;
        encIndexMax = encIndex + TASK_SIZE;
        free(task);

        for (; signIndex < signIndexMax; signIndex++)
        {
            signP = publicKeys + signIndex;
            for (; encIndex < encIndexMax; encIndex++)
            {
                encP = publicKeys + encIndex;
                calcRipe(mdctx, sha512md, ripemd160md, cache64, signP, encP);
                nlz = getNLZ(cache64, 20);
                if (nlz >= minExportThreshold)
                {
                    {
                        fprintf(stderr, "%ld, %ld, %ld\n", nlz, signIndex,
                                encIndex);
                    }
                }
                if (maxNLZ < nlz)
                {
                    maxNLZ = nlz;
                }
                calcRipe(mdctx, sha512md, ripemd160md, cache64, encP, signP);
                nlz = getNLZ(cache64, 20);
                if (nlz >= minExportThreshold)
                {
                    {
                        fprintf(stderr, "%ld, %ld, %ld\n", nlz, encIndex,
                                signIndex);
                    }
                }
                if (maxNLZ < nlz)
                {
                    maxNLZ = nlz;
                }
            }
        }
        printf("%zu->%zu, %zu->%zu : %zu\n", task->signkeyindex, signIndexMax,
               task->encryptkeyindex, encIndexMax, maxNLZ);
    }
    EVP_MD_CTX_free(mdctx);
    return NULL;
}

static int loadPublicKey(unsigned char *publicKey, const char *path)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    return loadKey1(publicKey, path, 64, 16777216);
}

/**
 * TODO: リファクタリング
 * TODO: 鍵キャッシュサーバー
 * TODO: 既存鍵を使ってアドレス探索
 * 制限付き同期キューでキューに詰める量を制限する
 */
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    publicKeys = calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
    if (publicKeys == NULL)
    {
        perror("calloc");
        return EXIT_FAILURE;
    }
    // 4362076160 == 65 * 16777216 * 4
    fprintf(stderr, "calloced\n");
    if (loadPublicKey(publicKeys, "publicKeys.bin") != 0)
    {
        free(publicKeys);
        return EXIT_FAILURE;
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

    QUEUE_DEFINE(queue);
    pthread_t prucude_thread;
    pthread_t consume_threads[THREAD_NUM];
    struct threadArg arg[THREAD_NUM];
    sha512md = EVP_sha512();
    ripemd160md = EVP_ripemd160();
    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        arg[i].publicKeys = publicKeys;
        arg[i].pubKeyNmemb = 0;
        arg[i].signBegin = LARGE_BLOCK_SIZE * (i / 4);
        arg[i].signEnd = LARGE_BLOCK_SIZE * ((i / 4) + 1);
        arg[i].encBegin = LARGE_BLOCK_SIZE * (i % 4);
        arg[i].encEnd = LARGE_BLOCK_SIZE * ((i % 4) + 1);
        arg[i].minExportThreshold = 4;
        arg[i].queue = &queue;
    }
    pthread_create(&prucude_thread, NULL, produce, &queue);
    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&consume_threads[i], NULL, consume, &arg[i]);
    }
    pthread_join(prucude_thread, NULL);
    for (size_t i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(consume_threads[i], NULL);
    }
    // shutdown:
    // free(privateKeys);
    free(publicKeys);
    return EXIT_SUCCESS;
}
