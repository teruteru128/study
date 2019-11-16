
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <gmp.h>

#if 0
uint32_t xor64(void) {
  static uint64_t x = 88172645463325252ULL;
  x = x ^ (x << 7);
  return x = x ^ (x >> 9);
}
#endif

uint32_t xor64(uint64_t *x) {
  uint64_t tmp = *x;
  tmp = tmp ^ (tmp << 7);
  return *x = tmp ^ (tmp >> 9);
}

#define FILENAME "/dev/urandom"
uint64_t readSeed(){
  FILE *f = NULL;
  f = fopen(FILENAME, "rb");
  if(f == NULL){
    perror("fopen");
    return 0;
  }
  uint64_t val = 0;
  size_t siz = fread(&val, sizeof(val), 1, f);
  if(siz != 1){
    fprintf(stderr, "failed random value(siz: %ld, %ld)\n", siz, sizeof(val));
  }
err:
  fclose(f);
  return val;
}

int main(int argc, char* argv[]){
  uint64_t seed = readSeed();
  mpz_t x, y, z, sum;
  mpz_inits(x, y, z, sum, NULL);
  mpz_set_si(x, -80538738812075974LL);
  mpz_set_si(y, 80435758145817515LL);
  mpz_set_si(z, 12602123297335631LL);

  printf("%08lx\n", seed);
  mpz_pow_ui(x, x, 3);
  mpz_pow_ui(y, y, 3);
  mpz_pow_ui(z, z, 3);
  mpz_add(sum, sum, x);
  mpz_add(sum, sum, y);
  mpz_add(sum, sum, z);
  gmp_printf("%Zd + %Zd + %Zd = %Zd\n", x,y,z, sum);

  return EXIT_SUCCESS;
}

