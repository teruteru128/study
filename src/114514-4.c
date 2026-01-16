
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>

/**
 * 114514*10^114508+114514*10^n+1は常に3の倍数
 */
int main(int argc, char const *argv[])
{
    mpz_t n, p, diff, term1, term2, power_n;
    mpz_inits(n, p, diff, term1, term2, power_n, NULL);
    mpz_set_ui(term1, 114514);
    mpz_ui_pow_ui(power_n, 10, 114508);
    mpz_mul(n, term1, power_n);
    mpz_nextprime(p, n);
    mpz_sub(diff, p, n);
    if (mpz_fits_ulong_p(diff) != 0)
    {
        unsigned long out = mpz_get_ui(diff);
        printf("prime: 114514*10^114508+%lu\n", out);
    }
    else
    {
        size_t length = mpz_sizeinbase(diff, 10) + 2;
        char *str = malloc(length);
        mpz_get_str(str, 10, diff);
        printf("prime: 114514*10^114508+%s\n", str);
        free(str);
    }
    mpz_clears(n, p, diff, term1, term2, power_n, NULL);
    return 0;
}
