
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "bitsieve.h"

#include <gmp.h>

#define BIT_LENGTH 262144

struct BitSieve
{
    unsigned long *bits;
    size_t bits_length;
    size_t length;
};

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
    mpz_init_set_ui(offset, 0U);

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
        gmp_fprintf(stderr, "\n+%Zd : xxx\n", offset);
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
        candidate = bs_retrieve(&searchSieve, &p, 1);
    }
    bs_free(&searchSieve);
    gmp_printf("%Zd\n", candidate);

    mpz_clears(p, candidate, initvalue, offset, NULL);
    return 0;
}
