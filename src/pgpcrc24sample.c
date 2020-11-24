
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRC24_INIT 0xB704CEL
#define CRC24_POLY 0x1864CFBL

typedef long crc24;
/**
 * @brief 
 * @author RFC-4880 section 6.1.
 * 
 * @param octets 
 * @param len 
 * @return crc24 
 */
crc24 crc_octets(unsigned char *octets, size_t len)
{
  crc24 crc = CRC24_INIT;
  int i;
  while (len--)
  {
    crc ^= (*octets++) << 16;
    for (i = 0; i < 8; i++)
    {
      crc <<= 1;
      if (crc & 0x1000000)
        crc ^= CRC24_POLY;
    }
  }
  return crc & 0xFFFFFFL;
}

int main(int argc, char *argv[])
{
  char text[] = "test";
  size_t len = strlen(text);
  crc24 crc = crc_octets((unsigned char *)text, len);
  printf("%06lx\n", crc);
  return EXIT_SUCCESS;
}
