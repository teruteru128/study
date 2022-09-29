
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gmp.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

uint64_t readSeed()
{
    uint64_t val = 0;
    ssize_t numberOfRandomBytes
        = getrandom(&val, sizeof(uint64_t), GRND_NONBLOCK);
    if (numberOfRandomBytes < 0)
    {
        return 0;
    }
    else
        return val;
}

int main(int argc, char *argv[])
{
    uint64_t seed = readSeed();
    mpz_t x, y, z, x3, y3, z3, sum;
    mpz_inits(x, y, z, x3, y3, z3, sum, NULL);
    mpz_set_str(x, "-80538738812075974", 10);
    mpz_set_str(y, "80435758145817515", 10);
    mpz_set_str(z, "12602123297335631", 10);

    printf("%08lx\n", seed);
    mpz_pow_ui(x3, x, 3);
    mpz_pow_ui(y3, y, 3);
    mpz_pow_ui(z3, z, 3);
    mpz_add(sum, sum, x3);
    mpz_add(sum, sum, y3);
    mpz_add(sum, sum, z3);
    gmp_printf("(%Zd)^3 + (%Zd)^3 + (%Zd)\n", x, y, z);
    gmp_printf(" = (%Zd) + (%Zd) + (%Zd)\n", x3, y3, z3);
    gmp_printf(" = %Zd\n", sum);
    mpz_clears(x, y, z, x3, y3, z3, sum, NULL);

    return EXIT_SUCCESS;
}
