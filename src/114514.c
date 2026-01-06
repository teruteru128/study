
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#define MAX_LENGTH 114514

static size_t getlength(MP_INT *n)
{
    size_t length;
    char *n_str = malloc(mpz_sizeinbase(n, 10) + 2);
    mpz_get_str(n_str, 10, n);
    length = strlen(n_str);
    free(n_str);
    return length;
}

int main(int argc, char const *argv[])
{
    MP_INT n;
    mpz_init(&n);
    // 指数
    int sisuu = 380404;
    // 仮数
    for (int kasuu = 3; kasuu <= 1000; kasuu++)
    {
        mpz_set_ui(&n, 2);
        mpz_mul_2exp(&n, &n, sisuu);
        mpz_mul_ui(&n, &n, kasuu);
        mpz_add_ui(&n, &n, 10);
        size_t length = getlength(&n);
        while (length > MAX_LENGTH)
        {
            sisuu--;
            mpz_set_ui(&n, 2);
            mpz_mul_2exp(&n, &n, sisuu);
            mpz_mul_ui(&n, &n, kasuu);
            mpz_add_ui(&n, &n, 10);
            length = getlength(&n);
        }
        printf("2^%d*%d+1: %zu digits\n", sisuu, kasuu, length);
    }
    mpz_clear(&n);
    return 0;
}
