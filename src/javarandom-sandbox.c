
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <gmp.h>

/**
 * java randomの逆演算とかをいじいじ
 */
int main(int argc, char const *argv[])
{
    uint64_t b = 0x5DEECE66DUL;
    uint64_t c = 0xDFE05BCB1365UL;
    printf("%" PRId64 "\n", b);
    printf("%" PRId64 "\n", c);
    printf("%016" PRIx64 "\n", (b * c));
    uint64_t d = 8868678245928342373ULL;
    printf("%016" PRIx64 "\n", d);
    printf("%016" PRIx64 "\n", b * d);
    mpz_t e, f, g, base;
    mpz_inits(e, f, g, base, NULL);
    mpz_set_ui(e, b);
    mpz_set_ui(f, d);
    mpz_mul(g, e, f);
    mpz_set_ui(base, 1);
    mpz_mul_2exp(base, base, 64);
    gmp_printf("before mask: %034Zx\n", g);
    mpz_mod(g, g, base);
    gmp_printf("after mask:  %034Zx\n", g);
    mpz_clears(e, f, g, base, NULL);
    return 0;
}
