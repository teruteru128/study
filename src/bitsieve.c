
// TODO: OpenSSLにあるBIGNUMへのサポートを視野に入れる
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "timeutil.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gmp.h>

#define PUBLISH_STRUCT_BS
#include "bitsieve.h"

static void bs_set(struct BitSieve *bs, size_t bitIndex)
{
    bs->bits[unitIndex(bitIndex)] |= bit(bitIndex);
}

static int bs_get(struct BitSieve *bs, size_t bitIndex)
{
    return ((bs->bits[unitIndex(bitIndex)] & bit(bitIndex)) != 0);
}

/**
 * @brief startから開始して最初にフラグの立っていない数を探します.
 *
 * @param bs
 * @param limit
 * @param start
 * @return int
 */
static size_t bs_sieveSearch(struct BitSieve *bs, size_t limit, size_t start)
{
    if (start >= limit)
        return (size_t)-1;

    size_t index = start;
    do
    {
        if (!bs_get(bs, index))
            return index;
        index++;
    } while (index < limit - 1UL);
    return (size_t)-1;
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

static struct BitSieve *smallSieve = NULL;

static void bs_smallSieve_Constract(void)
{
    smallSieve = bs_new();
    char *bs_file = getenv("SMALL_BITSIEVE_FILE");
    if (bs_file != NULL && access(bs_file, F_OK | R_OK) == 0)
    {
        FILE *fin = fopen(bs_file, "rb");
        if (fin == NULL)
        {
            perror("fopen small bitsieve file");
            exit(1);
        }
        bs_filein(smallSieve, fin);
        fclose(fin);
    }
    else
    {
        /*
         * このふるいの長さはJavaで実装されているものをそのまま流用しているため、最適化するには独自に実験して選択する必要があります。
         * 対象のbit
         * lengthが非常に長い場合、smallSieveの長さを大きくしても良いのかも？
         */
        // smallSieve.length = 1 * 64; // 64
        smallSieve->length = 150 * 64; // 9600
        // smallSieve.length = 500 * 64; // 32000
        // smallSieve.length = 512 * 64; // 32768
        // smallSieve.length = 780 * 64; // 49920
        // smallSieve.length = 1024 * 64; // 65536
        // smallSieve.length = 6554 * 64; // 419456
        // smallSieve.length = 8192 * 64; // 524288
        // smallSieve.length = 65536 * 64; // 4,194,304
        // smallSieve.length = 1048576 * 64; // 67,108,864
        // smallSieve.length = 67108864 * 64UL; // 4,294,967,296
        smallSieve->bits_length = unitIndex(smallSieve->length - 1) + 1;
        smallSieve->bits
            = calloc(smallSieve->bits_length, sizeof(unsigned long));
        if (smallSieve->bits == NULL)
        {
            perror("smallSieve.bits = calloc");
            exit(1);
        }

        bs_set(smallSieve, 0);
        size_t nextIndex = 1;
        size_t nextPrime = 3;

        /* エラトステネスの篩ループ */
        do
        {
            // nextPrimeの倍数を塗りつぶす
            bs_sieveSingle(smallSieve, smallSieve->length,
                           nextIndex + nextPrime, nextPrime);
            // nextIndexの次から探して塗りつぶされていない最初のindexを探す
            nextIndex = bs_sieveSearch(smallSieve, smallSieve->length,
                                       nextIndex + 1);
            nextPrime = 2 * nextIndex + 1;
        } while ((nextIndex != (size_t)-1)
                 && (nextPrime < smallSieve->length));
    }
}

static pthread_once_t smallSieveInitialize = PTHREAD_ONCE_INIT;

void bs_initSmallSieve()
{
    pthread_once(&smallSieveInitialize, bs_smallSieve_Constract);
}

struct BitSieve *bs_new(void)
{
    struct BitSieve *instance = calloc(1, sizeof(struct BitSieve));
    return instance;
}

/**
 * XXX: 初期化済みのBitSieveをbs_freeせずに再度初期化するとメモリリークが起きる
 * @brief
 *
 * @param bs
 * @param base
 * @param searchLen
 */
void bs_initInstance(struct BitSieve *bs, mpz_t *base, size_t searchLen)
{
    bs_initSmallSieve();
    if (bs == NULL || base == NULL)
    {
        return;
    }
    bs->length = searchLen;
    bs->bits_length = unitIndex(searchLen - 1) + 1;
    bs->bits = calloc(bs->bits_length, sizeof(unsigned long));
    size_t start = 0;

    size_t step = bs_sieveSearch(smallSieve, smallSieve->length, start);
    size_t convertedStep = ((step * 2) + 1);

    do
    {
        start = convertedStep - mpz_fdiv_ui(*base, convertedStep);
        if ((start & 1UL) == 0UL)
            start += convertedStep;
        bs_sieveSingle(bs, searchLen, (start - 1UL) / 2UL, convertedStep);

        step = bs_sieveSearch(smallSieve, smallSieve->length, step + 1UL);
        convertedStep = step * 2UL + 1UL;
    } while (step != (size_t)-1);
}

struct BitSieve *bs_getInstance(mpz_t *base, size_t searchLen)
{
    struct BitSieve *instance = bs_new();
    bs_initInstance(instance, base, searchLen);
    return instance;
}

void bs_free(struct BitSieve *bs)
{
    if (bs != NULL)
    {
        if (bs->bits != NULL)
        {
            memset(bs->bits, 0, sizeof(unsigned long) * bs->bits_length);
            free(bs->bits);
            bs->bits = NULL;
        }
        bs->bits_length = 0;
        bs->length = 0;
        free(bs);
    }
}

/**
 * XXX: gmp系とOpenSSL系で両方作るの？やだぁ……
 * TODO: mpz_probab_prime_p を非同期化してマルチスレッドにしたい
 * @brief Test probable primes in the sieve and return successful candidates.
 *
 * @see java.math.BitSieve#retrieve(BigInteger, int, Random)
 * @param p
 * @param offset
 * @param certainty
 * @return mpz_t*
 */
mpz_t *bs_retrieve(struct BitSieve *bs, mpz_t *initValue, int certainty)
{
    unsigned int offset = 1;
    mpz_t *candidate = malloc(sizeof(mpz_t));
    if (!candidate)
        exit(1);
    mpz_init(*candidate);
    for (size_t i = 0; i < bs->bits_length; i++)
    {
        unsigned long nextLong = ~bs->bits[i];
        for (int j = 0; j < 64; j++)
        {
            if ((nextLong & 1) == 1)
            {
                mpz_add_ui(*candidate, *initValue, offset);
                // mpz_millerrabin() でもいい可能性がある
                int answer = mpz_probab_prime_p(*candidate, certainty);
                if (answer == 1 || answer == 2)
                    return candidate;
                fprintf(stderr, "+%u: .\n", offset);
            }
            else
            {
                // initValue + offset was screened off by a bit sieve.
                fprintf(stderr, "+%u: x\n", offset);
            }
            nextLong >>= 1;
            offset += 2;
        }
    }
    mpz_clear(*candidate);
    free(candidate);
    return NULL;
}

void bs_foreach(struct BitSieve *bs,
                void (*function)(mpz_t *base, unsigned long offset, void *arg),
                mpz_t *base, void *arg)
{
    unsigned long offset = 1;
    for (size_t i = 0; i < bs->bits_length; i++)
    {
        unsigned long nextLong = ~bs->bits[i];
        for (int j = 0; j < 64; j++)
        {
            if ((nextLong & 1) == 1)
            {
                function(base, offset, arg);
            }
            nextLong >>= 1;
            offset += 2;
        }
    }
}

void bs_import() {}

/**
 * @brief
 *
 * @param out
 * @param outsize
 * @param bs
 * @return unsigned char*
 */
unsigned char *bs_export(unsigned char *out, size_t *outsize,
                         const struct BitSieve *bs)
{
    return NULL;
}

/**
 * @brief ビット篩をストリームへエクスポートする。
 * もしかしたら一つのストリームへ複数のビット篩がエクスポート可能かもしれない。
 * ほとんどすべての用途において必要ではないと思われるが。
 *
 * @param stream
 * @param bs
 * @return size_t
 */
size_t bs_fileout(FILE *stream, const struct BitSieve *bs)
{
    if (bs == NULL || stream == NULL)
        return 0;

    const size_t swaped_length = htobe64(bs->length);
    const size_t swaped_bits_length = htobe64(bs->bits_length);

    size_t sumofwritensize = 0;
    size_t writensize = 0;

    writensize = fwrite(&swaped_length, sizeof(size_t), 1, stream);
    if (writensize < 1)
    {
        perror("frwite");
        return 0;
    }
    sumofwritensize += writensize * sizeof(size_t);

    writensize = fwrite(&swaped_bits_length, sizeof(size_t), 1, stream);
    if (writensize < 1)
    {
        perror("frwite");
        return sumofwritensize + writensize * sizeof(size_t);
    }
    sumofwritensize += writensize * sizeof(size_t);

    const size_t bits_length = bs->bits_length;
#if 0

    unsigned long w = 0;
    for (size_t i = 0; i < bits_length; i++)
    {
        w = htobe64(bs->bits[i]);
        writensize = fwrite(&w, sizeof(unsigned long), 1, stream);
        if (writensize < 1)
        {
            perror("fwrite");
            return sumofwritensize + sizeof(unsigned long);
        }
        sumofwritensize += sizeof(unsigned long);
    }
#else
    unsigned long *work = calloc(sizeof(unsigned long), bits_length);
    memcpy(work, bs->bits, sizeof(unsigned long) * bits_length);

    for (size_t i = 0; i < bits_length; i++)
    {
        work[i] = htobe64(work[i]);
    }
    writensize = fwrite(work, sizeof(unsigned long), bits_length, stream);
    if (writensize < bits_length)
    {
        perror("fwrite");
    }
    free(work);
    sumofwritensize += sizeof(unsigned long) * writensize;
#endif

    return sumofwritensize;
}

size_t bs_filein(struct BitSieve *bs, FILE *stream)
{
    if (bs == NULL || stream == NULL)
        return 0;

    size_t sumofsize = 0;
    size_t readsize = 0;

    size_t swaped_length;
    readsize = fread(&swaped_length, sizeof(size_t), 1, stream);
    if (readsize < 1)
    {
        perror("fread");
        return 0;
    }
    sumofsize = readsize * sizeof(size_t);
    bs->length = be64toh(swaped_length);

    size_t swaped_bits_length;
    readsize = fread(&swaped_bits_length, sizeof(size_t), 1, stream);
    if (readsize < 1)
    {
        perror("fread");
        return sumofsize + readsize * sizeof(size_t);
    }
    sumofsize += readsize * sizeof(size_t);
    const size_t bits_length = bs->bits_length = be64toh(swaped_bits_length);

    bs->bits = calloc(bits_length, sizeof(unsigned long));
    /*
      早いのはどっち？
    */
#if 0
    unsigned long w = 0;
    for (size_t i = 0; i < bits_length; i++)
    {
        // 1個読んで
        readsize = fread(&w, sizeof(unsigned long), 1, stream);
        if (readsize < 1)
        {
            free(bs->bits);
            perror("fread");
            return sumofsize + sizeof(unsigned long);
        }
        // バイトオーダーを変換して書き込む
        bs->bits[i] = be64toh(w);
        sumofsize += sizeof(unsigned long);
    }
#else
    // 一括で全部読んで
    readsize = fread(bs->bits, sizeof(unsigned long), bits_length, stream);
    if (readsize < bits_length)
    {
        free(bs->bits);
        perror("fread");
        return sumofsize;
    }
    sumofsize += sizeof(unsigned long) * bits_length;
    for (size_t i = 0; i < bits_length; i++)
    {
        // あとからバイトオーダーを変換する
        bs->bits[i] = be64toh(bs->bits[i]);
    }
#endif

    return sumofsize;
}

size_t bs_getNextStep(struct bs_ctx *ctx)
{
    pthread_mutex_lock(&ctx->mutex);
    size_t returnstep
        = bs_sieveSearch(smallSieve, smallSieve->length, ctx->start);
    ctx->start = returnstep + 1UL;
    pthread_mutex_unlock(&ctx->mutex);
    return returnstep;
}
