
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

int main(int argc, char *argv[])
{
    FILE *fin1 = fopen("262144bit-initialValue1.txt", "r");
    FILE *fin2 = fopen("262144bit-prime1.txt", "w");
    if (fin1 == NULL || fin2 == NULL)
    {
        if (fin1 != NULL)
            fclose(fin1);
        if (fin2 != NULL)
            fclose(fin2);
        perror("fin");
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);
    mpz_inp_str(base, fin1, 16);
    fclose(fin1);
    mpz_add_ui(base, base, 40203);
    mpz_out_str(fin2, 16, base);
    fclose(fin2);

    return EXIT_SUCCESS;
}
