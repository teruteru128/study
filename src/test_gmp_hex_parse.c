
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

int main(int argc, char *argv[])
{
    mpz_t a;
    mpz_inits(a, NULL);
    mpz_set_str(a, "4041424344454647", 16);
    mpz_out_str(stdout, 16, a);
    mpz_clears(a, NULL);
    return 0;
}
