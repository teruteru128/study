
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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
    MP_INT min, max, window;

    mpz_init_set_ui(&min, 10);
    mpz_init_set_ui(&max, 10);
    mpz_pow_ui(&min, &min, digits - 1);
    mpz_pow_ui(&max, &max, digits);
    mpz_init(&window);
    mpz_sub(&window, &max, &min);

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

    MP_INT *array = malloc(sizeof(MP_INT) * num);
    for (size_t i = 0; i < num; i++)
    {
        MP_INT *ptr = array + i;
        mpz_init(ptr);
        mpz_urandomm(ptr, &state, &window);
        mpz_add(ptr, ptr, &min);
        mpz_nextprime(ptr, ptr);
    }
    gmp_randclear(&state);
    qsort(array, num, sizeof(__mpz_struct), (int (*)(const void *, const void *))mpz_cmp);
    for (size_t i = 0; i < num; i++)
    {
        size_t digits2 = mpz_sizeinbase(array + i, 10);
        if (digits != digits2)
        {
            fprintf(stderr, "digits not match: %zu, %zu\n", digits, digits2);
        }
        char *str = malloc(digits2 + 2);
        mpz_get_str(str, 10, array + i);
        printf("%s\n", str);
        fprintf(stderr, "%zu, %zu, %zu digits written\n", digits, digits2, strlen(str));
        free(str);
        mpz_clear(array + i);
    }
    free(array);
    mpz_clears(&min, &max, &window, NULL);
    return 0;
}
