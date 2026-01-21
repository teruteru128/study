
#include <stdio.h>
#include <gmp.h>

int main(int argc, char *argv[], char *envp[])
{
    mpz_t p, n, factor1, power_n, m, power_m;
    mpz_inits(p, n, factor1, power_n, m, power_m, NULL);
    for(int i = 5000; i <= 10000; i++)
    {
        mpz_set_ui(factor1, 2);
        mpz_ui_pow_ui(power_n, 10, i);
        mpz_sub_ui(power_n, power_n, 1);
        mpz_mul(factor1, factor1, power_n);
        mpz_divexact_ui(factor1, factor1, 3);
        mpz_sub_ui(factor1, factor1, 3);
        for(int j = 1; j < i; j++)
        {
           mpz_ui_pow_ui(power_m, 10, j); 
           mpz_sub(p, factor1, power_m);
           int r = mpz_probab_prime_p(p, 24);
           if(r){
               printf("%d, %d: %d\n", i, j, r);
           }
        }
    }
    mpz_clears(p, n, factor1, power_n, power_m, m, NULL);
    return 0;
}
