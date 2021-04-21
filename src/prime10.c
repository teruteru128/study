
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

/**
 * @brief 単一数について素数判定
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s [a]\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *fin1 = fopen(argv[1], "r");
    if (fin1 == NULL)
    {
        if (fin1 != NULL)
            fclose(fin1);
        perror("fin");
        return EXIT_FAILURE;
    }
    mpz_t primeNumberCandidate;
    mpz_init(primeNumberCandidate);
    mpz_inp_str(primeNumberCandidate, fin1, 16);
    fclose(fin1);

    int answer = mpz_probab_prime_p(primeNumberCandidate, 100);
    printf("%d\n", answer);
    mpz_clear(primeNumberCandidate);

    return EXIT_SUCCESS;
}
