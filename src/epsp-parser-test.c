
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
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

typedef struct string_array string_array;

typedef struct string_array{
  char** str;
  size_t length;
} string_array;

typedef struct string_list string_list;

typedef struct string_list{
  char* str;
  string_array* next;
} string_list;

/*
  文字列の配列を返す
*/
/*
int split(string_array** dest, regex_t pattern, char* input, size_t limit){
}
*/

int split(char **dest,char* src,char* delim)
{

}

int split_regex(string_array** dest, char* pattern, char* str, size_t limit){
}

int split_strtok(string_array** dest, char* delim, char* src, size_t limit){
}

/*
  プロトコルフォーマットに合致しているか検査しないといけないので面倒くさい可能性が高い
*/
int split_by_strtok(const char* str){
  char* target = strdup(str);

  free(target);
  return EXIT_FAILURE;
}

void free_string_array(string_array* str){
}

int split_by_regex(char* str, char* regex){
  regex_t regbuf;
  int errco=0;
  if((errco=regcomp(&regbuf, regex, REG_EXTENDED|REG_NEWLINE)) != 0){
    print_reg_error(errco, &regbuf);
    return 1;
  }

  regmatch_t match[8];
  size_t size = sizeof(match) / sizeof(regmatch_t);
  if((errco=regexec(&regbuf, str, size, match, 0)) != 0){
    print_reg_error(errco, &regbuf);
    regfree(&regbuf);
    return EXIT_FAILURE;
  }
  epsp_packet* packet = NULL;
  if(errco == REG_NOMATCH){
    //no match
    regfree(&regbuf);
    return 0;
  }
  packet = malloc(sizeof(epsp_packet));
  packet->code      = strtol(str + match[1].rm_so, NULL, 10);
  packet->hop_count = strtol(str + match[2].rm_so, NULL, 10);
  if(match[3].rm_so == -1 || match[3].rm_eo == -1){
    // not found
    packet->data = NULL;
  }else{
    size_t data_start = match[3].rm_so;
    size_t data_end = match[3].rm_eo;
    size_t data_len = data_end - data_start;
    packet->data = malloc(data_len + 1);
    memcpy(packet->data, str+data_start, data_len);
    packet->data[data_len] = 0;
  }
  free(packet->data);
  free(packet);

  for(size_t i = 0; i < size; i++){
    size_t start = match[i].rm_so;
    size_t end = match[i].rm_eo;
    if(start==-1||end==-1){
      fprintf(stderr, "no match : %ld\n", i);
      continue;
    }
    size_t len = end - start;
    printf("%ld : %ld, %ld , %ld: ", len, start, end, len);
    // substr ここから
    // strdupはない場合もある
    //char* group = strdup(str+start, len);
    // malloc + memset + strncpy_s 現環境にstrncpy_s無し
    //char* group = malloc(len+1);
    //memset(group, 0, len+1);
    //strncpy_s(group, len+1, str+start, len);
    // malloc + strncpy + null終端挿入
    //char* group = malloc(len+1);
    //strncpy(group, str+start, len);
    //group[len] = 0;
    // malloc + memcpy + null終端挿入
    char* group = malloc(len+1);
    memcpy(group, str+start, len);
    group[len] = 0;
    // substr ここまで
    printf("%s\n", group);
    free(group);
  }
  regfree(&regbuf);
  fprintf(stderr, "OK\n");
  return EXIT_SUCCESS;
}
/*
  初期化
  mainloop
    接続受け付け
  heartbeat loop
  aaa
*/
int main(int argc, char* argv[]){
  // 正規表現で分割 OR strtokで分割
  //
  char* regex = "([[:digit:]]+) ([[:digit:]]+) ?(.*)?";
  char* str = "116 12 25:6911:901:2";
  if(split_by_regex(str, regex) != EXIT_SUCCESS){
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

