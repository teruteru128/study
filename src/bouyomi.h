
#ifndef BOUYOMI_H
#define BOUYOMI_H 1

#include <stdint.h>
#include <inttypes.h>

#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)
#define DEFAULT_PORT 50001
#define DEFAULT_PORT_STR "50001"
#define DEFAULT_SERV_ADDRESS "localhost"
#define DEFAULT_SERV_ADDRESS4 "127.0.0.1"
#define DEFAULT_SERV_ADDRESS6 "::1"
#define ONION_SERV_ADDRESS "2ayu6gqru3xzfzbvud64ezocamykp56kunmkzveqmuxvout2yubeeuad.onion"

// アライメントが入るためそのまま送信してはいけない
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

typedef struct bouyomi_conf_t
{
  short command;
  short speed;
  short tone;
  short volume;
  short voice;
  char encode;
  size_t length;
  char *msg;
} bouyomi_conf;

typedef struct config_line_t
{
  char *key;
  char *value;
  struct config_line_t *next;
} config_line_t;

typedef enum charset_t
{
  UTF_8 = 0,
  UNICODE = 1,
  SHIFT_JIS = 2
} charset;

#endif
