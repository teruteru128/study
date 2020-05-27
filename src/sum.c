
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <gmp.h>
#include "random.h"


#define FILENAME "/dev/urandom"
uint64_t readSeed(){
  uint64_t val = 0;
  read_file(FILENAME, &val, sizeof(uint64_t), 1);
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

