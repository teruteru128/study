
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#define MAX_LENGTH 114514

static size_t getlength(MP_INT *n)
{
    char n_str[229038];
    mpz_get_str(n_str, 10, n);
    return strlen(n_str);
}

/**
 * 114514桁の素数を探索するための事前ツール
 */
int main(int argc, char const *argv[])
{
    MP_INT n;
    char n_str[229038];
    mpz_init(&n);
    // 指数
    int sisuu;
    // 仮数
    for (int kasuu = 1; kasuu <= 1000; kasuu++)
    {
        sisuu = 380408;
        mpz_set_ui(&n, 2);
        mpz_mul_2exp(&n, &n, sisuu);
        mpz_mul_ui(&n, &n, kasuu);
        mpz_add_ui(&n, &n, 1);
        mpz_get_str(n_str, 10, &n);
        size_t length = strlen(n_str);
        while (length >= MAX_LENGTH)
        {
            if (length == MAX_LENGTH)
            {
                printf("2^%d*%d+1: %zu digits\n", sisuu, kasuu, length);
            }
            sisuu--;
            mpz_set_ui(&n, 2);
            mpz_mul_2exp(&n, &n, sisuu);
            mpz_mul_ui(&n, &n, kasuu);
            mpz_add_ui(&n, &n, 1);
            mpz_get_str(n_str, 10, &n);
            length = strlen(n_str);
        }
        fprintf(stderr, "%d done\n", kasuu);
    }
    mpz_clear(&n);
    return 0;
}
