
#ifndef DUMP_NTP_PACKET_H
#define DUMP_NTP_PACKET_H

#include <stdio.h>
#include <stdint.h>

/* N_("NTP packet") */
struct NTP_Packet
{
  int32_t Control_Word;
  int32_t root_delay;
  int32_t root_dispersion;
  int32_t reference_identifier;
  int32_t reference_timestamp_seconds;
  int32_t reference_timestamp_fractions;
  int32_t originate_timestamp_seconds;
  int32_t originate_timestamp_fractions;
  int32_t receive_timestamp_seconds;
  int32_t receive_timestamp_fractions;
  int32_t transmit_timestamp_seconds;
  int32_t transmit_timestamp_fractions;
};

void dumpNTPpacket(struct NTP_Packet *packet, FILE *out);

#endif
