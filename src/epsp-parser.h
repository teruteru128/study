
#ifndef EPSP_PARSER_H
#define EPSP_PARSER_H

#include "string_list.h"
#include "string_array.h"

// TODO move to epsp_protocol.h
typedef struct epsp_packet_t{
  int code;
  int hop_count;
  char* code_str;
  char* hop_count_str;
  string_array* data;
} epsp_packet;

epsp_packet* epsp_packet_parse(char* line);

void epsp_packet_free(epsp_packet* packet);

// parser & unparser
// epsp_builder.h

#endif

