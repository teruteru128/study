
#include <gmp.h>
#include <stdio.h>

/**
 * 2^a+-(b)な素数のペアを探す
 * */
int main(int argc, char *argv[]) {
  mpz_t c, p, q, r, diff;
  mpz_inits(c, p, q, r, diff, NULL);
  mpz_setbit(c, 512);
  mpz_setbit(p, 512);
  mpz_setbit(p, 128);
  int i = 0;
  while (i < 10) {
    mpz_nextprime(p, p);
    mpz_sub(diff, p, c);

    mpz_sub(q, c, diff);

    if (mpz_probab_prime_p(q, 24) != 0) {
      mpz_mul(r, p, q);
      mpz_mul_ui(r, r, 1);
      if (mpz_probab_prime_p(r, 24) != 0) {
        gmp_printf("found! %Zd\n", r);
      }else{
        gmp_printf("not found, %Zu\n", diff);
      }
    }
  }
  return 0;
}
