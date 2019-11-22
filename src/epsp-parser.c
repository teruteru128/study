
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

epsp_packet* parse_epsp_packet(char* line){
  if(line_pattern == NULL || line == NULL){
    return NULL;
  }

  regmatch_t match[4];
  size_t size = sizeof(match)/sizeof(regmatch_t);
  int errorcode = regexec(line_pattern, line, size, match, 0);
  if(errorcode != 0){
    return NULL;
  }

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

  return packet;
}

