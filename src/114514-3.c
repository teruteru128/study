
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
    mpz_t p, term1, term2, power_n;
    mpz_inits(p, term1, term2, power_n, NULL);
    mpz_set_ui(term1, 114514);
    mpz_set_ui(term2, 114514);
    mpz_ui_pow_ui(power_n, 10, 114508);
    mpz_mul(term1, term1, power_n);
    int r = 0;
    for (int i = 1; i <= 114502; i++)
    {
        mpz_set(p, term1);

        mpz_set_ui(term2, 2);
        mpz_ui_pow_ui(power_n, 10, i);

        mpz_mul(term2, term2, power_n);

        mpz_add(p, p, term2);

        mpz_add_ui(p, p, 1);

        r = mpz_probab_prime_p(p, 24);
        printf("114514*10^114508+114514*10^%d+1 is %sprime\n", i, r != 0 ? "" : "not ");
    }
    mpz_clears(p, term1, term2, power_n, NULL);
    return 0;
}
