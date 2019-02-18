
#include <stdio.h>
#include <inttypes.h>
#include "random.h"

int main(int argc, char* argv[]){
  Random random;
  setSeed(&random, 0);
  printf("%u\n", next(&random, 32));
  printf("%u\n", nextInt(&random));
  printf("%u\n", nextIntWithRange(&random, 10));
  printf("%"PRIu64"\n", nextLong(&random));
}

