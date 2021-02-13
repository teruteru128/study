
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gmp.h>

#define BIT_LENGTH 262144

struct BitSieve
{
    long *bits;
    size_t bits_length;
    int length;
};

static struct BitSieve *smallSieve = NULL;

#define unitIndex(bitIndex) (bitIndex) >> 6

struct BitSieve *bs_getInstance(mpz_t *base, int searchLen)
{
    struct BitSieve *instance = calloc(1, sizeof(struct BitSieve));
    size_t unitNum = unitIndex(searchLen - 1) + 1;
    instance->bits = calloc(unitNum, sizeof(long));
    instance->bits_length = unitNum;
    instance->length = searchLen;
    return NULL;
}

mpz_t *retrieve(mpz_t p, unsigned int offset, int certainty)
{
    return NULL;
}

/**
 * @brief 
 * iv1 : 1
 * iv2 : 4035
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    FILE *fin = fopen("262144bit-initialValue2.txt", "r");
    mpz_t p;
    mpz_t candidate;
    mpz_t offset;
    mpz_inits(p, candidate, NULL);
    mpz_init_set_ui(offset, 4035);
    mpz_inp_str(p, fin, 16);
    fclose(fin);

    int searchLen = BIT_LENGTH / 20 * 64;
    struct BitSieve *sarchSieve;

    while (1)
    {
        mpz_add(candidate, p, offset);
        int answer = mpz_probab_prime_p(candidate, 1);
        if (answer == 1 || answer == 2)
        {
            gmp_printf("%Zd\n", candidate);
            break;
        }
        gmp_fprintf(stderr, "+%Zd : xxx\n", offset);
        mpz_add_ui(offset, offset, 2);
    }

    mpz_clears(p, candidate, offset, NULL);
    return 0;
}
