
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
  int64_t x = 8866128975287528LL;
  int64_t y = -8778405442862239LL;
  int64_t z = -2736111468807040LL;
  mpz_t x, y, z;
  mpz_inits(x, y, z, NULL);
  mpz_set_si(x, 8866128975287528LL);
  mpz_set_si(y, -8778405442862239LL);
  mpz_set_si(z, -2736111468807040LL);

  printf("%08lx\n", seed);
  printf("%ld\n", x * x * x + y * y * y + z * z * z);

  x = (((int64_t)xor64(&seed) << 32) + ((int64_t)xor64(&seed))) & 0x7FFFFFFFFFFFFFFFLL;
  y = -abs(xor64(&seed));
  z = -abs(xor64(&seed));
  printf("%ld, %ld, %ld\n", x, y, z);
  int64_t sum = 0;
  for(;y < 0; y++){
    for(;z < 0; z++){
      sum = y * y * y;
      sum += z * z * z;
      sum += x * x * x;
      if(0 < sum && sum < 100){
        printf("%ld\n", y);
        printf("%ld\n", sum);
      }
    }
  }
  return EXIT_SUCCESS;
}

