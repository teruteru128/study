
#include <stdio.h>
#include <gmp.h>

int main(int argc, char *argv[]) {
  mpz_t p, q, diff;
  mpz_inits(p, q, diff, NULL);
  mpz_init_set_str(p, "19124990244992669", 10);
  mpz_nextprime(q, p);
  mpz_sub(diff, q, p);
  gmp_printf("diff: %Zd\n", diff);
  mpz_clears(p, q, diff, NULL);
  return 0;
}
