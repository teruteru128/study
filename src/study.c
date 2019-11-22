
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "gettext.h"
#include "gettextsample.h"
#include "random.h"
#define D_SIZE (1024)


static const char *cmdline_ops[]={
  "orz",
  "yattaze",
  "Nabeatsu",
  "FizzBuzz",
  NULL
};

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

  (&printf)("  &printf = %p\n", &printf);
  printf("   printf = %p\n", printf);
  (*printf)("  *printf = %p\n", *printf);
  (**printf)(" **printf = %p\n", **printf);
  (***printf)("***printf = %p\n", ***printf);
  i = 1;
  int j;
  for(j=0;j<10;j++){
    printf("%d\n", !j);
  }
  while(i<argc){
    int is_command = 0;
    int j;
    for(j = 0; cmdline_ops[j] != NULL; j++){
      if(!strcmp(argv[i], cmdline_ops[j])){
        is_command=1;
      }
    }
  }
  return EXIT_SUCCESS;
}

