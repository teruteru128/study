
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include "rsa1024.h"

int main(int argc, char *argv[], char *envp[])
{
    if(argc < 2)
    {
        return 1;
    }

    mpz_t n;
    mpz_init_set_str(n, N, 16);
    FILE *in = fopen(argv[1], "r");
    if(in == NULL){
        perror("fopen");
        return 1;
    }
    char buffer[BUFSIZ];
    MP_INT *array = NULL;
    size_t array_size = 0;
    while(fgets(buffer, BUFSIZ, in))
    {
        MP_INT *tmp = realloc(array, (array_size + 1) * sizeof(MP_INT));
        mpz_init_set_str(tmp + array_size, buffer, 10);
        array_size++;
        array = tmp;
    }
    fclose(in);
    size_t i, j, i_max = array_size - 1;
    mpz_t sq1, sq2, diff, gcd;
    mpz_inits(sq1, sq2, diff, gcd, NULL);
    for(i = 0; i < array_size; i++)
    {
        mpz_pow_ui(sq1, array + i, 2UL);
        for(j = 0; j < array_size; j++)
        {
            if(mpz_cmp(array + i, array + j) <= 0)
            {
                continue;
            }
            mpz_pow_ui(sq2, array + j, 2UL);
            if(mpz_congruent_p(sq1, sq2, n) != 0) {
                mpz_sub(diff, array + i, array + j);
                mpz_gcd(gcd, diff, n);
                if(mpz_cmp_ui(gcd, 1) > 0 && mpz_cmp(gcd, n) < 0) {
                    gmp_printf("a! %Zd\n", gcd);
                }
                mpz_add(diff, array + i, array + j);
                mpz_gcd(gcd, diff, n);
                if(mpz_cmp_ui(gcd, 1) > 0 && mpz_cmp(gcd, n) < 0) {
                    gmp_printf("a! %Zd\n", gcd);
                }
            }
        }
    }
    mpz_clears(sq1, sq2, diff, gcd, NULL);
    mpz_clear(n);
    return 0;
}

