
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>
#include <unistd.h>
#include <gmp.h>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    size_t digits = strtoull(argv[1], NULL, 10);
    size_t num = (argc < 3) ? 1 : strtoull(argv[2], NULL, 10);
    mpz_t n;
    mpz_init(n);
    mpz_t min, max, window;
    
    mpz_init_set_ui(min, 10);
    mpz_init_set_ui(max, 10);
    mpz_pow_ui(min, min, digits - 1);
    mpz_pow_ui(max, max, digits);
    mpz_init(window);
    mpz_sub(window, max, min);
    gmp_randstate_t state;
    gmp_randinit_default(state);
    mpz_t seed;
    mpz_init(seed);
    uint8_t op[128];
    size_t length = 128;
    getrandom(op, length, GRND_NONBLOCK);
    mpz_import(seed, length, 1, 1, 0, 0, op);
    gmp_randseed(state, seed);
    mpz_clear(seed);
    for (size_t i = 0; i < num; i++)
    {
        mpz_urandomm(n, state, window);
        mpz_add(n, n, min);
        mpz_nextprime(n, n);
        size_t digits2 = mpz_sizeinbase(n, 10);
        if (digits != digits2)
        {
            fprintf(stderr, "digits not match: %zu, %zu\n", digits, digits2);
        }
        char *str = malloc(digits2 + 2);
        mpz_get_str(str, 10, n);
        printf("%s\n", str);
        free(str);
    }
    gmp_randclear(state);
    mpz_clears(min, max, window, NULL);
    return 0;
}
