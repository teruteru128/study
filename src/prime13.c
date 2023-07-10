
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gmp.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PUBLISH_STRUCT_BS
#include "bitsieve.h"
#include "queue.h"

QUEUE_DEFINE(primes);

struct arg
{
    size_t length;
    __mpz_struct *base;
    struct bs_ctx ctx;
};

static void bs_set(struct BitSieve *bs, size_t bitIndex)
{
    size_t unitIndex = unitIndex(bitIndex);
    bs->bits[unitIndex] |= bit(bitIndex);
}

/**
 * @brief startの倍数にフラグを立てます.
 *
 * @param bs
 * @param limit
 * @param start
 * @param step
 */
static void bs_sieveSingle(struct BitSieve *bs, size_t limit, size_t start,
                           size_t step)
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
static void *consume(void *arg)
{
    struct arg *argp = (struct arg *)arg;
    struct BitSieve *bs = bs_new();
    bs->length = argp->length;
    bs->bits_length = unitIndex((argp->length) - 1) + 1;
    bs->bits = calloc(bs->bits_length, sizeof(uint64_t));
    __mpz_struct *base = argp->base;
    struct bs_ctx *ctx = &argp->ctx;
    size_t start = 0;
    size_t step = bs_getNextStep(ctx);
    size_t convertedStep = ((step * 2) + 1);
    do
    {
        start = convertedStep - mpz_fdiv_ui(base, convertedStep);
        if ((start & 1UL) == 0UL)
            start += convertedStep;
        bs_sieveSingle(bs, bs->length, (start - 1UL) / 2UL, convertedStep);
        step = bs_getNextStep(ctx);
        convertedStep = step * 2UL + 1UL;
    } while (step != (size_t)-1);
    return bs;
}

#define THREADS 12

/**
 * @brief ビット篩の初期化をマルチスレッド化してみる
 * 初期化っていうか生成
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

    __mpz_struct base;
    mpz_init(&base);
    FILE *fin = fopen(argv[1], "r");
    if (fin == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
    size_t size = mpz_inp_str(&base, fin, 16);
    printf("ロードしました。%lu\n", size);
    fclose(fin);
    fin = NULL;
    bs_initSmallSieve();

    const size_t searchLength = mpz_sizeinbase(&base, 2) / 20 * 64;

    struct arg arg;
    arg.length = searchLength;
    arg.base = &base;
    arg.ctx.start = 0;
    pthread_mutex_init(&arg.ctx.mutex, NULL);

    pthread_t *consumer_threads = calloc(THREADS, sizeof(pthread_t));
    for (size_t i = 0; i < THREADS; i++)
    {
        pthread_create(consumer_threads + i, NULL, consume, &arg);
    }

    struct BitSieve *result[THREADS];
    for (size_t i = 0; i < THREADS; i++)
    {
        pthread_join(consumer_threads[i], (void **)(result + i));
    }
    mpz_clear(&base);
    free(consumer_threads);
    struct BitSieve bs = { 0 };
    bs.length = searchLength;
    bs.bits_length = unitIndex(searchLength - 1) + 1;
    bs.bits = calloc(bs.bits_length, sizeof(unsigned long));
    const size_t l = bs.bits_length;
    size_t j = 0;
    for (size_t i = 0; i < THREADS; i++)
    {
        for (j = 0; j < l; j++)
        {
            bs.bits[j] |= result[i]->bits[j];
        }
    }

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
