
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "%s [a] [b]\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *fin1 = fopen(argv[1], "r");
    FILE *fin2 = fopen(argv[2], "r");
    if (fin1 == NULL || fin2 == NULL)
    {
        if (fin1 != NULL)
            fclose(fin1);
        if (fin2 != NULL)
            fclose(fin2);
        perror("fin");
        return EXIT_FAILURE;
    }
    mpz_t initV, prime1, sub;
    mpz_inits(initV, prime1, sub, NULL);
    mpz_inp_str(initV, fin1, 16);
    mpz_inp_str(prime1, fin2, 16);
    fclose(fin1);
    fclose(fin2);

    mpz_sub(sub, prime1, initV);
    gmp_printf("%Zd\n", sub);
    mpz_clears(initV, prime1, sub, NULL);

    return EXIT_SUCCESS;
}
