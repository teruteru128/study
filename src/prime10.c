
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

int main(int argc, char *argv[])
{
    FILE *fin1 = fopen("262144bit-prime1.txt", "r");
    if (fin1 == NULL)
    {
        if (fin1 != NULL)
            fclose(fin1);
        perror("fin");
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);
    mpz_inp_str(base, fin1, 16);
    fclose(fin1);

    int answer = mpz_probab_prime_p(base, 100);
    printf("%d\n", answer);

    return EXIT_SUCCESS;
}
