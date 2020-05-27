
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <regex.h>
#include "epsp-parser.h"
#include "string_array.h"
#include "string_list.h"

#define LINE_PATTERN "([[:digit:]]+) ([[:digit:]]+) ?(.+)?"

static regex_t* line_pattern = NULL;

void parser_init(void){
  if(line_pattern == NULL){
    line_pattern = malloc(sizeof(regex_t));
    if(line_pattern==NULL){
      perror("line_pattern malloc failed");
    }
    int errorcode = regcomp(line_pattern, LINE_PATTERN, REG_EXTENDED|REG_NEWLINE);
  }

}

void parser_free(void){
  if(line_pattern != NULL){
    regfree(line_pattern);
    line_pattern = NULL;
  }
}

/**
  failed : false(0)
success : true(1)
*/
static int parse_internal(epsp_packet* packet, const char* line){
  // 一応nullチェック
  if(packet == NULL){
    return 0;
  }
  // 作業用コピー copy_lineはfree
  char* copy_line = strdup(line);
  // 予め改行文字(CRLF)は削除する
  // 半角スペースで分割する
  char* code_str = NULL;
  char* hop_str = NULL;
  char* data_str = NULL;
  char* trash_str = NULL;
  {
    char* catch = NULL;
    code_str = strtok_r(copy_line, " ", &catch);
    hop_str = strtok_r(NULL, " ", &catch);
    data_str = strtok_r(NULL, " ", &catch);
    trash_str = strtok_r(NULL, " ", &catch);
  }
  // 分割した結果0個以下または4個以上の場合失敗
  if(code_str == NULL){
    // 0個
    goto clean;
  }
  if(trash_str != NULL){
    // 4個以上
    goto clean;
  }

  // 初期値
  packet->code = -1;
  packet->code_str = NULL;
  packet->hop_count = -1;
  packet->hop_count_str = NULL;
  packet->data = NULL;

  {
    char* catch = NULL;
    packet->code = strtol(code_str, &catch, 10);
    if(*catch != '\0'){
      goto clean;
    }
    packet->code_str = strdup(code_str);
    if(packet->code_str == NULL){
      goto clean;
    }
    if(hop_str != NULL){
      packet->hop_count = strtol(hop_str, &catch, 10);
      if(*catch != '\0'){
        goto clean;
      }
      packet->hop_count_str = strdup(hop_str);
      if(packet->hop_count_str == NULL){
        free(packet->code_str);
        goto clean;
      }
    }
  }
  if(data_str != NULL){
    string_array * data = string_array_split(data_str, ":");
    if(data == NULL){
      perror("");
      return EXIT_FAILURE;
    }
    packet->data = data;
  }else{
  }
  free(copy_line);
  return 1;

clean:
  free(copy_line);
  return 0;
}

void epsp_packet_free(epsp_packet* packet)
{

}

/**
    https://github.com/teruteru128/epsp-peer-cs/blob/master/Client/Common/Net/Packet.cs
*/
epsp_packet* epsp_packet_parse(char* line){
  if(line_pattern == NULL || line == NULL){
    return NULL;
  }

  // allocate
  // tryparse(call internal parser)
  // return packet OR Exception
  epsp_packet* packet = malloc(sizeof(epsp_packet));
  memset(packet, 0, sizeof(epsp_packet));
  if(parse_internal(packet, line)){
  }else{
    //failed
    epsp_packet_free(packet);
    return NULL;
  }
}

/*
 * https://p2pquake.github.io/epsp-specifications/epsp-specifications.html
 */
void print_reg_error(int errorcode, regex_t* buf);

/*
  文字列の配列を返す
  https://hg.openjdk.java.net/jdk/jdk12/file/06222165c35f/src/java.base/share/classes/java/lang/String.java#l2274
*/
/*
int split(string_array** dest, regex_t pattern, char* input, size_t limit){
}
*/

string_list* split(char* src, char* regex, int limit)
{
  char ch = 0;
  size_t regex_len = strlen(regex);
  if(((regex_len == 1 && strchr(".$|()[{^?*+\\", ch=regex[0])==NULL)||
  (regex_len == 2 && regex[0] =='\\' && (((ch=regex[1])-'0')|('9'-ch))<0&&((ch-'a')|('z'-ch)) < 0 &&((ch-'A')|('Z'-ch)) < 0))){
    int off=0;
    int next = 0;
    int limited = limit > 0;
    string_list* list = malloc(sizeof(string_list));
    while((next = regex[off]) != '\0'){
      if(!limited||string_list_size(list)<limit-1){
        char*s=malloc(next-off+1);
        strncpy(s, regex, next-off);
        s[next-off]=0;
        string_list_add(list, s);
      }else{}
    }
  }
  return NULL;
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
  return EXIT_SUCCESS;
}
