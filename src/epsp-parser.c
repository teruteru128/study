
//
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "epsp-parser.h"

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
  char* code_str = strtok(copy_line, " ");
  char* hop_str = strtok(NULL, " ");
  char* data_str = strtok(NULL, " ");
  char* trash_str = strtok(NULL, " ");
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
  packet->hop_count = -1;
  packet->data = NULL;

  char* catch = NULL;
  packet->code = strtol(code_str, &catch, 10);
  if(*catch != '\0'){
    goto clean;
  }
  if(hop_str != NULL){
    packet->hop_count = strtol(hop_str, &catch, 10);
    if(*catch != '\0'){
      goto clean;
    }
  }
  if(data_str != NULL){
    // 本当は分割して配列として渡したい
    packet->data = strdup(data_str);
  }
  free(copy_line);
  return 1;

  /*
  regmatch_t match[4];
  size_t size = sizeof(match)/sizeof(regmatch_t);
  int errorcode = regexec(line_pattern, line, size, match, 0);
  if(errorcode != 0){
    return NULL;
  }
  // データをコード、ホップ数、その他データにパースする。失敗したらその時点で終了
  epsp_packet* packet = malloc(sizeof(epsp_packet));
  packet->code = strtol(line+match[1].rm_so, NULL, 0);
  packet->hop_count = strtol(line+match[2].rm_so, NULL, 0);
  if(match[3].rm_so == -1 || match[3].rm_eo == -1){
    packet->data = NULL;
  }else{
    size_t data_start = match[3].rm_so;
    size_t data_end = match[3].rm_eo;
    size_t data_len = data_end - data_start;
    packet->data = malloc(data_len + 1);
    memcpy(packet->data, line+data_start, data_len);
    packet->data[data_len] = 0;
  }
  */
clean:
  free(copy_line);
  return 0;
}

/**
    https://github.com/teruteru128/epsp-peer-cs/blob/master/Client/Common/Net/Packet.cs
*/
epsp_packet* parse_epsp_packet(char* line){
  if(line_pattern == NULL || line == NULL){
    return NULL;
  }

  // allocate
  // tryparse(call internal parser)
  // return packet OR Exception
  epsp_packet* packet = malloc(sizeof(epsp_packet));
  if(parse_internal(packet, line)){
  }else{
    //failed
    epsp_packet_free(packet);
    return NULL;
  }
}

