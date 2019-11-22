
#ifndef EPSP_PARSER_H
#define EPSP_PARSER_H

// TODO move to epsp_protocol.h
typedef struct epsp_packet_t{
  int code;
  int hop_count;
  char* data;
} epsp_packet;

// epsp_builder.h

#endif

