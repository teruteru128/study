
#include <gmp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  mpz_t n;
  mpz_t p;
  mpz_inits(n, p, NULL);
  mpz_fib_ui(n, 2603732);
  mpz_set_ui(p, 3);
  for (int i = 0; i < 20000000; i++) {
    if (mpz_divisible_p(n, p)) {
      unsigned long r = mpz_remove(n, n, p);
      gmp_printf("%Zd^%lu\n", p, r);
    }
  }
  mpz_clears(n, p, NULL);
  return 0;
}
