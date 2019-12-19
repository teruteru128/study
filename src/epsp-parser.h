
#ifndef EPSP_PARSER_H
#define EPSP_PARSER_H

#include "string_list.h"

// TODO move to epsp_protocol.h
typedef struct epsp_packet_t{
  int code;
  int hop_count;
  size_t data_length;
  char** data;
  string_list* data_ptr;
} epsp_packet;

epsp_packet* epsp_packet_parse(char* line);

void epsp_packet_free(epsp_packet* packet);

// parser & unparser
// epsp_builder.h

#endif

