
#ifndef BOUYOMI_H
#define BOUYOMI_H 1

#include <stdint.h>
#include <inttypes.h>

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)
#define DEFAULT_PORT 50001
#define DEFAULT_PORT_STR "50001"
#define DEFAULT_SERV_ADDRESS "localhost"
#define DEFAULT_SERV_ADDRESS_2 "192.168.11.3"
#define DEFAULT_SERV_ADDRESS4 "127.0.0.1"
#define DEFAULT_SERV_ADDRESS6 "::1"
#define ONION_SERV_ADDRESS "2ayu6gqru3xzfzbvud64ezocamykp56kunmkzveqmuxvout2yubeeuad.onion"

/**
 * アライメントが入るためそのまま送信してはいけない
 */
typedef struct bouyomi_header_t
{
  short command;
  short speed;
  short tone;
  short volume;
  short voice;
  char encode;
  char empty; // alignment
  int32_t length;
} bouyomi_header;

/**
 * see: https://github.com/0x75960/bouyomi/blob/28d01a9/src/lib.rs
 */
struct bouyomi_conf
{
  char *host;
  char *port;
};

struct bouyomi_talk_conf
{
  short command;
  short speed;
  short tone;
  short volume;
  short voice;
  char encode;
};

struct config_line_t;
struct config_line_t
{
  char *key;
  char *value;
  struct config_line_t *next;
};

typedef enum charset
{
  UTF_8 = 0,
  UNICODE = 1,
  SHIFT_JIS = 2
} charset_t;

struct args
{
  int ignore_errors;
  int loglevel;
  char *servAddr;
  char *servPortStr;
};

#endif
