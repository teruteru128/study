
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include "gettextsample.h"

static const char *cmdline_ops[]={
  "orz",
  "yattaze",
  "Nabeatsu",
  "FizzBuzz",
  NULL
};

int main(int argc, char* argv[]){
  helloWorld001();
  (&printf)("  &printf = %p\n", &printf);
  printf("   printf = %p\n", printf);
  (*printf)("  *printf = %p\n", *printf);
  (**printf)(" **printf = %p\n", **printf);
  (***printf)("***printf = %p\n", ***printf);
  int i = 1;
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

