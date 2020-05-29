
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

int main(int argc, char *argv[])
{
  mpz_t n, nfac, nfacadd1, nfacsub1;
  mpz_inits(n, nfac, nfacadd1, nfacsub1, NULL);
  mpz_set_ui(n, 1);
  mpz_set_ui(nfac, 1);
  unsigned int i = 1;
  int isPrime = 0;
  char msg[3][17] = {"not prime", "probably prime", "definitely prime"};
  for (i = 1; i <= 16384; i++)
  {
    mpz_mul_ui(nfac, nfac, i);
    if (i <= 2048)
    {
      continue;
    }
    mpz_add_ui(nfacadd1, nfac, 1);
    isPrime = mpz_probab_prime_p(nfacadd1, 20);
    printf("%d! + 1 is %s\n", i, msg[isPrime]);
    mpz_sub_ui(nfacsub1, nfac, 1);
    isPrime = mpz_probab_prime_p(nfacsub1, 20);
    printf("%d! - 1 is %s\n", i, msg[isPrime]);
  }
  return EXIT_SUCCESS;
}
