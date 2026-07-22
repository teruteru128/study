
#include <gmp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  mpz_t base, n, p1, p2;
  mpz_inits(base, n, p1, NULL);
  mpz_ui_pow_ui(base, 10, 60000);
  mpz_set_ui(p2, 3);
  for (int j = 0; j < 2000; j++) {
    mpz_set_ui(p1, 2);
    mpz_sub_ui(n, base, j * 2 + 1);
    for (int i = 0; i < 2000; i++) {
      if (mpz_divisible_p(n, p1)) {
        gmp_printf("%d !: %Zu\n", j * 2 + 1, p1);
      }
      mpz_nextprime(p1, p1);
    }
  }
  mpz_clears(base, n, p1, p2, NULL);
  return 0;
}
