
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <endian.h>
#include "epsp-parser.h"

/*
 * https://p2pquake.github.io/epsp-specifications/epsp-specifications.html
 */
void print_reg_error(int errorcode, regex_t* buf){
    size_t len = regerror(errorcode, buf, NULL, 0);
    char* msg = (char*)calloc(len, sizeof(char));
    regerror(errorcode, buf, msg, len);
    printf("%s\n", msg);
    free(msg);
}

typedef struct epsp_packet_t{
  int code;
  int hop_count;
  char* data;
} epsp_packet;

/* strlen(src)+1バイトの領域を新たに割り当てしディープコピー */
char* strclone(const char* src){
  size_t len = strlen(src);
  return strnclone(src, len);
}

/*
   max(strlen(src)+1, n)バイトの領域を新たに割り当てしディープコピー
   strdupを使うといい string.h
 */
char* strnclone(const char* src, const size_t n){
  char* dest = calloc(len+1, sizeof(char));
  return strncpy(dest, src, len);
}

int main(int argc, char* argv[]){
  // 正規表現で分割 OR strtokで分割
  //
  char* regex = "([[:digit:]]+) ([[:digit:]]+) ?(.*)?";
  regex_t regbuf;
  int errco=0;
  if((errco=regcomp(&regbuf, regex, REG_EXTENDED|REG_NEWLINE)) != 0){
    print_reg_error(errco, &regbuf);
    return 1;
  }

  regmatch_t match[8];
  size_t size = sizeof(match) / sizeof(regmatch_t);
  char* str = "116 1 25:6911:901:2";
  if((errco=regexec(&regbuf, str, size, match, 0)) != 0){
    print_reg_error(errco, &regbuf);
    regfree(&regbuf);
    return EXIT_FAILURE;
  }
  for(size_t i = 0; i < size; i++){
    size_t start = match[i].rm_so;
    size_t end = match[i].rm_eo;
    if(start==-1||end==-1){
      //printf("no match\n");
      continue;
    }
    size_t len = end - start;
    printf("%ld : ", len);
    // substr ここから
    char* group = calloc(len+1, sizeof(char));
    strncpy(group, str+start, len);
    // substr ここまで
    printf("%s\n", group);
    free(group);
  }
  regfree(&regbuf);
  fprintf(stderr, "OK\n");
  return EXIT_SUCCESS;
}

