
#ifndef EPSP_PARSER_H
#define EPSP_PARSER_H

typedef struct string_array string_array;

typedef struct string_array{
  char** str;
  size_t length;
} string_array;

typedef struct string_node string_node;

typedef struct string_node {
  char* data;
  string_node* next;
  string_node* prev;
} string_node;

typedef struct string_list string_list;

/*https://ja.wikipedia.org/wiki/%E9%80%A3%E7%B5%90%E3%83%AA%E3%82%B9%E3%83%88#%E7%B7%9A%E5%BD%A2%E3%83%AA%E3%82%B9%E3%83%88_2*/
typedef struct string_list{
  string_node* firstNode;
  string_node* lastNode;
} string_list;

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

