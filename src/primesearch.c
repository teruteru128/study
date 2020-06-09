
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

static char *getMessage(int isprime)
{
  char *msg = NULL;
  if (isprime == 0)
  {
    msg = "is not prime";
  }
  else if (isprime == 1)
  {
    msg = "is probably prime";
  }
  else if (isprime == 2)
  {
    msg = "is definitely prime";
  }
  else
  {
    msg = "error!";
  }
  return msg;
}

int main(int argc, char *argv[])
{
  mpz_t num;
  mpz_t p;
  mpz_t one;
  //mpz_inits(num, p, one,NULL);
  mpz_init2(one, 8192);
  mpz_init2(num, 8192);
  mpz_init2(p, 8192);
  mpz_setbit(one, 0);
  int bitlen = 8193;
  mpz_setbit(num, bitlen);
  int i;
  int j;
  //printf("%lu\n", mpz_sizeinbase(num, 2));
  //printf("%s\n", mpz_odd_p(num) != 0 ? "odd" : "even");
  int isprime = 0;

  for (i = 8191; i <= bitlen; i++)
  {
    mpz_set(num, one);
    mpz_setbit(num, i);
    for (j = 1; j < i; j++)
    {
      mpz_set(p, num);
      mpz_setbit(p, j);
      isprime = mpz_probab_prime_p(p, 15);
      switch (isprime)
      {
      case 0:
        //printf("is not prime(%d)\n", i);
        break;
      case 1:
      case 2:
        printf("%s(i: %d, j: %d)\n", getMessage(isprime), i, j);
        break;
      }
    }
    fflush(stdout);
  }
  mpz_clears(one, num, p, NULL);
  return EXIT_SUCCESS;
}
