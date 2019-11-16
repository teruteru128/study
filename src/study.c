
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "gettext.h"
#include "gettextsample.h"
#include "random.h"
#define D_SIZE (1024)


int main(int argc, char* argv[]){
  helloWorld001();
  char *path = "/dev/urandom";
  int64_t seed;
  FILE* fp = NULL;

  fp = fopen(path, "rb");
  if(fp == NULL){
    perror("fopen");
    return EXIT_FAILURE;
  }

  size_t len = 0;
  if((len = fread(&seed, sizeof(int64_t), 1, fp)) < 1){
    perror("fread");
    return EXIT_FAILURE;
  }

  if(fclose(fp)){
    perror("fclose");
    return EXIT_FAILURE;
  }

  Random random;
  setSeed(&random, seed);

  double d=0;
  int gotGoldenClover = 0;
  size_t count = 0;

  int i, c = nextIntWithRange(&random, 128);

  for(i=0;i<c;i++){
     //xor();
  }

  do{
    d=nextDoubleXor();
    gotGoldenClover = d < 0.0007;
    count++;
    printf("%ld : %d %lf\n", count, gotGoldenClover, d);
  }while(!gotGoldenClover);

  return EXIT_SUCCESS;
}

