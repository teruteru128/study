
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/random.h>
#include <gmp.h>
#include <inttypes.h>

int main(int argc, char const *argv[])
{
    mpz_t n;
    mpz_t base;
    mpz_t p;
    mpz_t gcd;
    mpz_t r;
    __gmp_randstate_struct state;
    gmp_randinit_default(&state);
    MP_INT seed;
    mpz_init(&seed);
    uint8_t op[128];
    size_t length = 128;
    getrandom(op, length, GRND_NONBLOCK);
    mpz_import(&seed, length, 1, 1, 0, 0, op);
    gmp_randseed(&state, &seed);
    mpz_clear(&seed);
    mpz_init_set_str(n, "1173862644527310602810772458945853336289531284067733422883739235027023883895418786543239159745282093976101916023532579587467260026189256371450858936839758115134243949245764521704178576393", 10);
    mpz_init(base);
    mpz_init(p);
    mpz_init(gcd);
    mpz_init(r);
    mpz_pow_ui(base, n, 4);
    mpz_urandomm(p, &state, n);
    uint64_t count = 0;
    while (1)
    {
        mpz_powm_ui(r, p, 2, base);
        mpz_gcd(gcd, r, n);
        if (mpz_cmp_ui(gcd, 1) != 0)
        {
            break;
        }
        mpz_set(p, r);
        count++;
        if ((count % 1000000) == 0)
        {
            fprintf(stderr, "count: %" PRIu64 "\n", count);
        }
    }
    size_t gcdlength = mpz_sizeinbase(gcd, 10);
    char *str = malloc(gcdlength);
    printf("%s\n", str);
    free(str);
    mpz_clear(n);
    mpz_clear(base);
    mpz_clear(p);
    mpz_clear(gcd);
    gmp_randclear(&state);
    return 0;
}
