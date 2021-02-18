
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <gmp.h>

#define BIT_LENGTH 262144

#define unitIndex(bitIndex) ((bitIndex) >> 6)
#define bit(bitIndex) (1UL << ((bitIndex) & ((1 << 6) - 1)))

struct BitSieve
{
    unsigned long *bits;
    size_t bits_length;
    size_t length;
};

static void bs_set(struct BitSieve *bs, size_t bitIndex)
{
    size_t unitIndex = unitIndex(bitIndex);
    bs->bits[unitIndex] |= bit(bitIndex);
}

static int bs_get(struct BitSieve *bs, size_t bitIndex)
{
    size_t unitIndex = unitIndex(bitIndex);
    return ((bs->bits[unitIndex] & bit(bitIndex)) != 0);
}

static int bs_sieveSearch(struct BitSieve *bs, size_t limit, size_t start)
{
    if (start >= limit)
        return -1;

    size_t index = start;
    do
    {
        if (!bs_get(bs, index))
            return (int)index;
        index++;
    } while (index < limit - 1);
    return -1;
}

static void bs_sieveSingle(struct BitSieve *bs, size_t limit, size_t start, size_t step)
{
    while (start < limit)
    {
        bs_set(bs, start);
        start += step;
    }
}

static struct BitSieve smallSieve;

static void bs_smallSieve_Constract(void)
{
    smallSieve.length = 150 * 64;
    smallSieve.bits_length = unitIndex(smallSieve.length - 1) + 1;
    smallSieve.bits = calloc(smallSieve.bits_length, sizeof(unsigned long));

    bs_set(&smallSieve, 0);
    size_t nextIndex = 1;
    size_t nextPrime = 3;

    do
    {
        bs_sieveSingle(&smallSieve, smallSieve.length, nextIndex + nextPrime, nextPrime);
        nextIndex = (size_t)bs_sieveSearch(&smallSieve, smallSieve.length, nextIndex + 1);
        nextPrime = 2 * nextIndex + 1;
    } while ((nextIndex > 0) && (nextPrime < smallSieve.length));
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

struct BitSieve *bs_getInstance(mpz_t *base, size_t searchLen)
{
    pthread_once(&once_control, bs_smallSieve_Constract);
    struct BitSieve *instance = calloc(1, sizeof(struct BitSieve));
    instance->bits_length = unitIndex(searchLen - 1) + 1;
    instance->bits = calloc(instance->bits_length, sizeof(unsigned long));
    instance->length = searchLen;
    size_t start = 0;

    size_t step = (size_t)bs_sieveSearch(&smallSieve, smallSieve.length, start);
    unsigned long convertedStep = (unsigned long)((step * 2) + 1);

    mpz_t b, q;
    mpz_init_set(b, *base);
    mpz_init(q);
    do
    {
        start = mpz_mod_ui(q, b, convertedStep);
        start = convertedStep - start;
        if (start % 2 == 0)
            start += convertedStep;
        bs_sieveSingle(instance, searchLen, (start - 1) / 2, convertedStep);

        step = (size_t)bs_sieveSearch(&smallSieve, smallSieve.length, step + 1);
        convertedStep = (step * 2) + 1;
    } while (step > 0);
    mpz_clears(b, q, NULL);
    return instance;
}

void bs_initInstance(struct BitSieve *bs, mpz_t *base, size_t searchLen)
{
    pthread_once(&once_control, bs_smallSieve_Constract);
    bs->bits_length = unitIndex(searchLen - 1) + 1;
    bs->bits = calloc(bs->bits_length, sizeof(unsigned long));
    bs->length = searchLen;
    int start = 0;

    int step = bs_sieveSearch(&smallSieve, smallSieve.length, (size_t)start);
    int convertedStep = ((step * 2) + 1);

    mpz_t b, q;
    mpz_init_set(b, *base);
    mpz_init(q);
    do
    {
        start = (int)mpz_fdiv_r_ui(q, b, (unsigned long)convertedStep);
        start = convertedStep - start;
        if (start % 2 == 0)
            start += convertedStep;
        bs_sieveSingle(bs, searchLen, (size_t)((start - 1) / 2), (size_t)convertedStep);

        step = bs_sieveSearch(&smallSieve, smallSieve.length, (size_t)(step + 1));
        convertedStep = (step * 2) + 1;
    } while (step > 0);
    mpz_clears(b, q, NULL);
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
    }
}

/**
 * XXX: gmp系とOpenSSL系で両方作るの？やだぁ……
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
    mpz_init(*candidate);
    for (size_t i = 0; i < bs->bits_length; i++)
    {
        unsigned long nextLong = ~bs->bits[i];
        for (int j = 0; j < 64; j++)
        {
            if ((nextLong & 1) == 1)
            {
                mpz_add_ui(*candidate, *initValue, offset);
                int answer = mpz_probab_prime_p(*candidate, certainty);
                if (answer == 1 || answer == 2)
                    return candidate;
                fputs(".", stderr);
            }
            else
            {
                // initValue + offset was screened off by a bit sieve.
                fputs("x", stderr);
            }
            fflush(stderr);
            nextLong >>= 1;
            offset += 2;
        }
    }
    mpz_clear(*candidate);
    free(candidate);
    return NULL;
}

/**
 * size_t とかについてる `_t` ってtypedefを表してたんですね
 * iv1 : 1
 * iv2 : 12777
 * @brief prime6 <primefile> [offset, default=1]
 * 
 * @see https://hg.openjdk.java.net/jdk8/jdk8/jdk/file/687fd7c7986d/src/share/classes/java/math/BigInteger.java#l736
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    mpz_t p, initvalue;
    mpz_t offset;
    mpz_inits(p, initvalue, NULL);
    mpz_init_set_ui(offset, 19754);

    FILE *fin = fopen("262144bit-initialValue2.txt", "r");
    if (fin == NULL)
    {
        mpz_clears(p, initvalue, offset, NULL);
        perror("fopen");
        return EXIT_FAILURE;
    }
    mpz_inp_str(initvalue, fin, 16);
    fclose(fin);
    mpz_set(p, initvalue);
    mpz_add(p, p, offset);

    mpz_set_ui(offset, 0);

    unsigned int searchLen = BIT_LENGTH / 20 * 64;
    struct BitSieve searchSieve;
    bs_initInstance(&searchSieve, &p, (size_t)searchLen);
    mpz_t *candidate = bs_retrieve(&searchSieve, &p, 100);

    while ((candidate == NULL) || (mpz_sizeinbase(*candidate, 2) != BIT_LENGTH))
    {
        mpz_sub(offset, p, initvalue);
        gmp_printf("+%Zd : xxx\n", offset);
        mpz_add_ui(p, p, searchLen * 2U);
        if (mpz_sizeinbase(p, 2) != BIT_LENGTH)
        {
            fputs("bitsize was broken!\n", stderr);
            break;
        }
        mpz_clrbit(p, 0);
        bs_free(&searchSieve);
        bs_initInstance(&searchSieve, &p, searchLen);
        mpz_clear(*candidate);
        free(candidate);
        candidate = bs_retrieve(&searchSieve, &p, 100);
    }
    gmp_printf("%Zd\n", candidate);

    mpz_clears(p, candidate, initvalue, offset, NULL);
    return 0;
}
