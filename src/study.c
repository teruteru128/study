
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "gettext.h"
#include "gettextsample.h"
#include "random.h"
#define D_SIZE (1024)

static void print_reg_error(int errorcode, regex_t* buf){
    size_t len = regerror(errorcode, buf, NULL, 0);
    char* msg = malloc(len);
    regerror(errorcode, buf, msg, len);
    fprintf(stderr, "%s\n", msg);
    free(msg);
}

static const char *cmdline_ops[]={
  "orz",
  "yattaze",
  "Nabeatsu",
  "FizzBuzz",
  NULL
};

int main(int argc, char* argv[]){

  char* p = " ";
  regex_t ptn;
  int errorcode = regcomp(&ptn, p, REG_EXTENDED|REG_NEWLINE);
  if(errorcode != 0){
    print_reg_error(errorcode, &ptn);
    return EXIT_FAILURE;
  }

  regmatch_t match[5];
  size_t size = sizeof(match)/ sizeof(regmatch_t);

  char* string = "111111 22 333";

  errorcode = regexec(&ptn, string, size, match, 0);
  if(errorcode != 0){
    print_reg_error(errorcode, &ptn);
    return EXIT_FAILURE;
  }

  printf("match[0] -> so : %d, eo : %d\n", match[0].rm_so, match[0].rm_eo);
  int i = 0;
  for(;i < 5;i++){
    printf("%d, %d\n", match[i].rm_so, match[i].rm_eo);
  }
  if(match[1].rm_so == -1 || match[4].rm_so != -1){
    fputs("packet failed", stderr);
  }
  printf("%s\n", &string[match[1].rm_so]);
  printf("%s\n", &string[match[2].rm_so]);
  printf("%s\n", &string[match[3].rm_so]);

  return EXIT_SUCCESS;
}

