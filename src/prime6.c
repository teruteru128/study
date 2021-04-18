
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define PUBLISH_STRUCT_BS
#include "bitsieve.h"

#include <gmp.h>

#define BIT_LENGTH 262144
#define SEARCH_LENGTH (BIT_LENGTH / 20 * 64)
#define DEFAULT_CERTAINTY 1

/**
 * size_t とかについてる `_t` ってtypedefを表してたんですね
 * iv1 : 0
 * iv2 : 35470
 * @brief prime6 <primefile> [offset, default=1]
 * 
 * @see https://hg.openjdk.java.net/jdk8/jdk8/jdk/file/687fd7c7986d/src/share/classes/java/math/BigInteger.java#l736
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    mpz_t p, initValue;
    mpz_t offset;
    mpz_inits(p, initValue, offset, NULL);

    FILE *fin = fopen("262144bit-initialValue2.txt", "r");
    if (fin == NULL)
    {
        mpz_clears(p, initValue, offset, NULL);
        perror("fopen");
        return EXIT_FAILURE;
    }
    mpz_inp_str(initValue, fin, 16);
    fclose(fin);
    mpz_set(p, initValue);
    mpz_add_ui(p, p, 44008U +8546U);

    struct BitSieve searchSieve;
    bs_initInstance(&searchSieve, &p, (size_t)SEARCH_LENGTH);
    mpz_t *candidate = bs_retrieve(&searchSieve, &p, DEFAULT_CERTAINTY);

    while ((candidate == NULL) || (mpz_sizeinbase(*candidate, 2) != BIT_LENGTH))
    {
        mpz_sub(offset, p, initValue);
        gmp_fprintf(stderr, "\n+%Zd : xxx\n", offset);
        mpz_add_ui(p, p, SEARCH_LENGTH * 2U);
        if (mpz_sizeinbase(p, 2) != BIT_LENGTH)
        {
            fputs("bitsize was broken!\n", stderr);
            break;
        }
        mpz_clrbit(p, 0);
        bs_free(&searchSieve);
        bs_initInstance(&searchSieve, &p, SEARCH_LENGTH);
        mpz_clear(*candidate);
        free(candidate);
        candidate = bs_retrieve(&searchSieve, &p, DEFAULT_CERTAINTY);
    }
    bs_free(&searchSieve);
    if (candidate != NULL)
    {
        gmp_printf("%Zd\n", *candidate);

        mpz_clear(*candidate);
        free(candidate);
    }
    mpz_clears(p, initValue, offset, NULL);

    return EXIT_SUCCESS;
}
