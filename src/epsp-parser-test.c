
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "epsp-parser.h"
#include <regex.h>
#include <errno.h>

int main(int argc, char* argv[]){
  char* regex = "(\\d{3}) (\\d+)(?: (.+))?";
  regex_t regbuf;
  int errco=0;
  if((errco=regcomp(&regbuf, regex, REG_EXTENDED|REG_NEWLINE)) != 0){
    perror("regcomp");
    size_t len = regerror(errco, &regbuf, NULL, 0);
    char* msg = (char*)calloc(len, sizeof(char));
    regerror(errco, &regbuf, msg, len);
    printf("%s\n", msg);
    free(msg);
    return 1;
  }

  regmatch_t match[8];
  size_t size = sizeof(match) / sizeof(regmatch_t);
  if((errco=regexec(&regbuf, "116 1 25:6911:901:2", size, match, 0)) != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    perror("regexec");
    size_t len = regerror(errco, &regbuf, NULL, 0);
    char* msg = (char*)calloc(len, sizeof(char));
    regerror(errco, &regbuf, msg, len);
    printf("%s\n", msg);
    free(msg);
    regfree(&regbuf);
    return EXIT_FAILURE;
  }
  for(size_t i = 0; i < size; i++){
    size_t startindex = match[i].rm_so;
    size_t endindex = match[i].rm_eo;
    if(startindex==-1||endindex==-1){
      printf("no match\n");
      continue;
    }
    printf("%ld\n", endindex - startindex);
  }
  regfree(&regbuf);
  fprintf(stderr, "OK\n");
  return EXIT_SUCCESS;
}

