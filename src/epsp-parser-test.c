
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include "epsp-parser.h"

/*
  https://p2pquake.github.io/epsp-specifications/epsp-specifications.html
  初期化
  mainloop
    接続受け付け
  heartbeat loop
*/
int main(int argc, char* argv[]){
  /*
  // 正規表現で分割 OR strtokで分割
  //
  char* regex = "([[:digit:]]+) ([[:digit:]]+) ?(.*)?";
  char* str = "116 12 25:6911:901:2";
  if(split_by_regex(str, regex) != EXIT_SUCCESS){
    return EXIT_FAILURE;
  }
  */
  return EXIT_SUCCESS;
}

