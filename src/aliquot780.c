
#include <gmp.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

int main(int argc, char const *argv[]) {
  mpz_t n;
  mpz_t base;
  mpz_t p;
  mpz_t gcd;
  mpz_t r;
  mpz_t diff;
  __gmp_randstate_struct state;
  gmp_randinit_default(&state);
  MP_INT seed;
  mpz_init(&seed);
  uint8_t op[128];
  size_t length = 128;
  getrandom(op, length, GRND_NONBLOCK);
  mpz_import(&seed, length, 1, 1, 0, 0, op);
  gmp_randseed(&state, &seed);
  mpz_clear(&seed);
  mpz_init_set_str(
      n,
      "117386264452731060281077245894585333628953128406773342288373923502702388"
      "389541878654323915974528209397610191602353257958746726002618925637145085"
      "8936839758115134243949245764521704178576393",
      10);
  mpz_init(base);
  mpz_init(p);
  mpz_init_set_ui(gcd, 1);
  mpz_init(r);
  mpz_init(diff);
  mpz_pow_ui(base, n, 4);
  mpz_urandomm(p, &state, n);
  mpz_urandomm(r, &state, n);
  uint64_t count = 0;
  while (mpz_cmp_ui(gcd, 1) == 0) {
      // ポラードのロー法的ななにか
    mpz_sub(diff, p, r);
    mpz_abs(diff, diff);
    mpz_gcd(gcd, diff, n);
    if (mpz_cmp_ui(gcd, 1) != 0) {
      break;
    }
    count++;
    if ((count % 1000000) == 0) {
      fprintf(stderr, "count: %" PRIu64 "\n", count);
    }
    mpz_powm_ui(p, p, 2, n);
    mpz_powm_ui(r, r, 3, n);
  }
  if (mpz_cmp(gcd, n)) {
    fprintf(stderr, "fail!\n");
  } else {
    size_t gcdlength = mpz_sizeinbase(gcd, 10);
    char *str = malloc(gcdlength);
    printf("%s\n", str);
    free(str);
  }
  mpz_clear(n);
  mpz_clear(base);
  mpz_clear(p);
  mpz_clear(gcd);
  gmp_randclear(&state);
  return 0;
}
