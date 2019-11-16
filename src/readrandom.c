
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
#if 0
#define D_SIZE (786432)
#else
#define D_SIZE (1024)
#endif

/*
    if(useTrue == 1){
      use /dev/random
    }else{
      use /dev/urandom
    } calloc
*/
size_t readrandom(void *buf, size_t size, size_t nmemb, int useTrue){
  
  return 0;
}

int main(int argc, char* argv[]){
  char* path = "/dev/urandom";
  char buf1[D_SIZE];
  FILE* fp = NULL;

  fp = fopen(path, "rb");
  if(fp == NULL){
    perror("fopen");
    return EXIT_FAILURE;
  }

  size_t len = 0;
  if((len = fread(buf1, sizeof(char), D_SIZE, fp)) < D_SIZE){
    perror("fread");
    return EXIT_FAILURE;
  }

  if(fclose(fp)){
    perror("fclose");
    return EXIT_FAILURE;
  }
  char* base64 = base64encode(buf1, len);
  size_t length = strlen(base64);
  size_t unit = length / 4;
  printf("length : %lu, unit : %lu\n", length, unit);
  printf("%s\n", base64);
  return EXIT_SUCCESS;
}
