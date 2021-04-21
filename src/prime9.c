
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

int main(int argc, char *argv[])
{
    FILE *fin = fopen("524288bit-7d7a92f9-0a35-4cb2-bc1f-fd0e43486e61-initialValue.txt", "r");
    FILE *fout = fopen("524288bit-7d7a92f9-0a35-4cb2-bc1f-fd0e43486e61-prime.txt", "w");
    if (fin == NULL || fout == NULL)
    {
        if (fin != NULL)
            fclose(fin);
        if (fout != NULL)
            fclose(fout);
        perror("fin");
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    mpz_add_ui(base, base, 191319);
    mpz_out_str(fout, 16, base);
    fclose(fout);

    return EXIT_SUCCESS;
}
