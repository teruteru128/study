
#ifndef GETTIME_H
#define GETTIME_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>

#define DEFAULT_SERVER "time.google.com"

#define OFFSET 2208988800ULL

typedef struct sntp_t
{
  unsigned int li : 2;
  unsigned int vn : 3;
  unsigned int mode : 3;
  unsigned int stratum : 8;
  unsigned int poll : 8;
  unsigned int precison : 8;
  unsigned int root_delay;
  unsigned int root_dispresion;
  unsigned int reference_identifire;
  unsigned long reference_timestamp;
  unsigned long originate_timestamp;
  unsigned long recive_timestamp;
  unsigned long transmit_timestamp;
  // Key Identifier
  // Message Digest
} SNTP;

#endif
