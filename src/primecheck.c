
#include <gmp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  mpz_t base, n, m, l, p1, p2;
  mpz_inits(base, n, m, l, p1, NULL);
  mpz_ui_pow_ui(m, 10, 50);
  mpz_sub_ui(m, m, 33);
  mpz_pow_ui(m, m, 12);
  mpz_sub_ui(m, m, 1);
  mpz_pow_ui(m, m, 125);
  mpz_sub_ui(m, m, 1);
  mpz_ui_pow_ui(l, 10, 50);
  mpz_sub_ui(l, l, 33);
  mpz_pow_ui(l, l, 12);
  mpz_sub_ui(l, l, 1);
  mpz_pow_ui(l, l, 25);
  mpz_sub_ui(l, l, 1);
  mpz_divexact(n, m, l);
  mpz_set_ui(p1, 2);
  for(int i = 0; i < 1000000; i++) {
    if(mpz_divisible_p(n, p1)){
        gmp_printf("%Zu\n", p1);
    }
    mpz_nextprime(p1, p1);
  }
  mpz_clears(base, n, m, l, p1, p2, NULL);
  return 0;
}
