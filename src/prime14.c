
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[])
{
    BitSieve small = { 0, 0, NULL };
    small.length = 0x40000000UL * 64;
    small.bits_length = unitIndex(small.length - 1) + 1;
    printf("%lu, %lu\n", small.bits_length, small.bits_length * sizeof(unsigned long));
    small.bits = calloc(small.bits_length, sizeof(unsigned long));

    bs_set(&small, 0);

    size_t nextIndex = 1;
    size_t nextPrime = 3;

    do
    {
        bs_sieveSingle(&small, small.length, nextIndex + nextPrime, nextPrime);

        nextIndex = bs_sieveSearch(&small, small.length, nextIndex + 1);
        nextPrime = 2 * nextIndex + 1;
    } while ((nextIndex != (size_t)-1) && (nextPrime < small.length));

    FILE *fout = fopen("0x1000000000smallsieve.bs", "wb");
    if(fout == NULL)
    {
        perror("fopen");
        bs_free(&small);
        return EXIT_FAILURE;
    }
    bs_fileout(fout, &small);
    fclose(fout);
    bs_free(&small);

    return EXIT_SUCCESS;
}
