
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include "epsp-parser.h"
#include "string_array.h"

/*
 * https://p2pquake.github.io/epsp-specifications/epsp-specifications.html
  https://p2pquake.github.io/epsp-specifications/epsp-specifications.html
  初期化
  mainloop
    接続受け付け
  heartbeat loop
 */
void print_reg_error(int errorcode, regex_t* buf){
    size_t len = regerror(errorcode, buf, NULL, 0);
    char* msg = (char*)calloc(len, sizeof(char));
    regerror(errorcode, buf, msg, len);
    printf("%s\n", msg);
    free(msg);
}

/*
   max(strlen(src)+1, n)バイトの領域を新たに割り当てしディープコピー
   strdupを使うといい string.h
 */
char* strnclone(const char* src, const size_t n){
  char* dest = calloc(n+1, sizeof(char));
  return strncpy(dest, src, n);
}

/* strlen(src)+1バイトの領域を新たに割り当てしディープコピー */
char* strclone(const char* src){
  size_t len = strlen(src);
  return strnclone(src, len);
}
/*
  文字列の配列を返す
*/
int main(int argc, char* argv[]){
  /*
  // 正規表現で分割 OR strtokで分割
  //
  char* regex = "([[:digit:]]+) ([[:digit:]]+) ?(.*)?";
  char* str = "116 12 25:6911:901:2";
  char** packets={
    "211",
    "232",
    "233",
    "234",
    "235",
    "236",
    "237",
    "238",
    "239",
    "243",
    "244",
    "246",
    "247",
    "248",
    "291",
    "292",
    "293",
    "295",
    "298",
    "299",
    "551",
    "552",
    "555",
    "556",
    "561",
    "611",
    "612",
    "614",
    "615",
    "631",
    "632",
    "634",
    "694",
    NULL
  };
  if(split_by_regex(str, regex) != EXIT_SUCCESS){
    return EXIT_FAILURE;
  }
  */
  return EXIT_SUCCESS;
}

