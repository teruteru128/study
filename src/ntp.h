
#ifndef DUMP_NTP_PACKET_H
#define DUMP_NTP_PACKET_H

#include <stdio.h>
#include <stdint.h>

#define OFFSET 2208988800UL

/**
 * @brief  N_("NTP packet") 
 * 
 */
struct NTP_Packet
{
  uint32_t Control_Word;
  uint32_t root_delay;
  uint32_t root_dispersion;
  uint32_t reference_identifier;
  uint32_t reference_timestamp_seconds;
  uint32_t reference_timestamp_fractions;
  uint32_t originate_timestamp_seconds;
  uint32_t originate_timestamp_fractions;
  uint32_t receive_timestamp_seconds;
  uint32_t receive_timestamp_fractions;
  uint32_t transmit_timestamp_seconds;
  uint32_t transmit_timestamp_fractions;
};

void dumpNTPpacket(struct NTP_Packet *packet, FILE *out);

#endif
