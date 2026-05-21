
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    mpz_t a;
    mpz_t b;
    mpz_t exp;
    mpz_t base;
    mpz_inits(a, b, exp, base, NULL);
    mpz_set_ui(a, 2);
    mpz_set_ui(exp, 4292);
    mpz_set_ui(base, 246864461023UL);
    mpz_pow_ui(a, a, 4292 + 123432230511UL);
    mpz_mul_ui(a, a, 21181);
    mpz_add_ui(a, a, 1);
    mpz_mod(b, a, base);
    size_t len = mpz_sizeinbase(b, 10) + 2;
    char *str = malloc(len);
    mpz_get_str(str, 10, b);
    printf("%s\n", str);
    free(str);
    mpz_clears(a, b, exp, base, NULL);
    return 0;
}
