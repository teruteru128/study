
#include <stdio.h>
#include <gmp.h>

int main(int argc, char *argv[], char *envp[])
{
    mpz_t n, p, nAdd1;
    mpz_inits(n, p, nAdd1, NULL);
    mpz_init_set_ui(n, 2);
    mpz_set_str(p, "1000000000000000000", 10);
    for(size_t i = 0; ; i++)
    {
        mpz_nextprime(p, p);
        mpz_mul(n, n, p);
        mpz_add_ui(nAdd1, n, 1);
        int r = mpz_probab_prime_p(nAdd1, 24);
        if(r)
        {
            gmp_printf("%zu, %Zd\n", i, nAdd1);
            break;
        }
        if((i + 1) % 1000 == 0)
        {
            fprintf(stderr, "%zu\n", i);
        }
    }
    mpz_clears(n, p, nAdd1, NULL);
    return 0;
}

