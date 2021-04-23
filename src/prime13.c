
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#include <pthread.h>
#define PUBLISH_STRUCT_BS
#include "bitsieve.h"
#include "queue.h"
#define unitIndex(bitIndex) ((bitIndex) >> 6)
#define bit(bitIndex) (1UL << ((bitIndex) & ((1 << 6) - 1)))

QUEUE_DEFINE(primes);

/**
 * @brief BitSieveのbits配列における1要素に対して1つのmutexが対応する配列
 * 
 */
static pthread_mutex_t *bits_mutexes;

struct arg
{
    struct BitSieve *bs;
    mpz_t *base;
    struct bs_ctx *ctx;
};

static void bs_set(struct BitSieve *bs, size_t bitIndex)
{
    size_t unitIndex = unitIndex(bitIndex);
    pthread_mutex_lock(bits_mutexes + unitIndex);
    bs->bits[unitIndex] |= bit(bitIndex);
    pthread_mutex_unlock(bits_mutexes + unitIndex);
}

/**
 * @brief startの倍数にフラグを立てます.
 * 
 * @param bs 
 * @param limit 
 * @param start 
 * @param step 
 */
static void bs_sieveSingle(struct BitSieve *bs, size_t limit, size_t start, size_t step)
{
    while (start < limit)
    {
        bs_set(bs, start);
        start += step;
    }
}

/**
 * @brief 篩がけスレッド
 * 
 * 共有アイテム
 * small sieve
 * bitsieve.bits[]
 * bits_mutex[]
 * 
 * @param arg 
 * @return void* 
 */
void *consume(void *arg)
{
    struct arg *argp = (struct arg *)arg;
    struct BitSieve *bs = argp->bs;
    mpz_t *base = argp->base;
    struct bs_ctx *ctx = argp->ctx;
    size_t start = 0;
    size_t step = bs_getNextStep(ctx);
    size_t convertedStep = ((step * 2) + 1);
    do
    {
        start = convertedStep - mpz_fdiv_ui(*base, convertedStep);
        if ((start & 1UL) == 0UL)
            start += convertedStep;
        bs_sieveSingle(bs, bs->length, (start - 1UL) / 2UL, convertedStep);
        step = bs_getNextStep(ctx);
        convertedStep = step * 2UL + 1UL;
    } while (step != (size_t)-1);
    return NULL;
}

/**
 * @brief ビット篩の初期化をマルチスレッド化してみる
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(const int argc, const char *argv[])
{
    if (argc < 2)
    {
        return EXIT_FAILURE;
    }

    mpz_t base;
    mpz_init(base);
    FILE *fin = fopen(argv[1], "r");
    if (fin == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
    size_t size = mpz_inp_str(base, fin, 16);
    printf("ロードしました。%lu\n", size);
    fclose(fin);
    fin = NULL;
    bs_initSmallSieve();

    const size_t searchLength = mpz_sizeinbase(base, 2) / 20 * 64;

    struct BitSieve bs = {0};
    bs.length = searchLength;
    bs.bits_length = unitIndex(searchLength - 1) + 1;
    bs.bits = calloc(bs.bits_length, sizeof(unsigned long));

    bits_mutexes = calloc(bs.bits_length, sizeof(pthread_mutex_t));
    for (size_t i = 0; i < bs.bits_length; i++)
    {
        pthread_mutex_init(bits_mutexes + i, NULL);
    }

    struct bs_ctx ctx = {0, PTHREAD_MUTEX_INITIALIZER};

    struct arg arg;
    arg.base = &base;
    arg.bs = &bs;
    arg.ctx = &ctx;

    pthread_t *consumer_threads = calloc(12, sizeof(pthread_t));
    for (size_t i = 0; i < 12; i++)
    {
        pthread_create(consumer_threads + i, NULL, consume, &arg);
    }

    for (size_t i = 0; i < 12; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }
    mpz_clear(base);
    free(consumer_threads);

    for (size_t i = 0; i < bs.bits_length; i++)
    {
        pthread_mutex_destroy(bits_mutexes + i);
    }
    free(bits_mutexes);

    char outfilename[FILENAME_MAX] = "";
    {
        // 拡張子書き換え
        char *work = strdup(argv[1]);
        char *dot = strrchr(work, '.');
        if (dot != NULL)
        {
            *dot = '\0';
        }
        snprintf(outfilename, FILENAME_MAX, "%s.bs", work);
        free(work);
    }
    FILE *fout = fopen(outfilename, "wb");
    bs_fileout(fout, &bs);
    fclose(fout);
    bs_free(&bs);
    return EXIT_SUCCESS;
}
