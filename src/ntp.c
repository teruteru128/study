
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ntp.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include <time.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <math.h>

void dumpNTPpacket(struct NTP_Packet *packet, FILE *out)
{
  // NULL check
  if (!packet || !out)
    return;

  unsigned char *a = (unsigned char *)packet;
  for (size_t i = 0; i < sizeof(struct NTP_Packet); i++)
  {
    fprintf(out, "%02x", a[i]);
    if ((i % 16) == 7)
    {
      fprintf(out, " ");
    }
    if ((i % 16) == 15)
    {
      fprintf(out, "\n");
    }
  }

  uint32_t w = bswap_32(packet->Control_Word);
  int leap_indicator = (w >> 30) & 0x03;
  int version_number = (w >> 27) & 0x07;
  int mode = (w >> 24) & 0x07;
  int stratum = (w >> 16) & 0xff;
  char poll = (w >> 8) & 0xff;
  char pre = (char)(w >> 0) & 0xff;
  int root_delay = bswap_32(packet->root_delay);
  int root_dispersion = bswap_32(packet->root_dispersion);
  int reference_identifier = packet->reference_identifier;

  fprintf(out, "LI : %d\n", leap_indicator);
  fprintf(out, "VN : %d\n", version_number);
  fprintf(out, "mode : %d\n", mode);
  fprintf(out, "Stratum : %d\n", stratum);
  fprintf(out, "Poll : %d(%d)\n", poll, 1 << poll);
  fprintf(out, "Precision : %d(%.32f)\n", pre, pow(2, pre));
  fprintf(out, "Root Delay : %d(%f)\n", root_delay, root_delay / 0x1p+16);
  fprintf(out, "Root Dispersion : %d(%f)\n", root_dispersion, root_dispersion / 0x1p+16);
  fprintf(out, "Reference ID : %08x", bswap_32(reference_identifier));

  if (stratum == 1)
  {
    char refid[5] = {0};
    memcpy(refid, &reference_identifier, 4);
    fprintf(out, "(%s)", refid);
  }
  else if (stratum >= 2)
  {
    if (version_number == 3)
    {
      struct in_addr ad = {htonl(reference_identifier)};
      char *addrstr = inet_ntoa(ad);
      if (addrstr)
        fprintf(out, "(%s)", addrstr);
    }
    else if (version_number == 4)
    {
      fprintf(out, "(*´ω｀*)");
    }
  }
  fprintf(out, "\n");
  uint32_t reference_timestamp_seconds = bswap_32(packet->reference_timestamp_seconds);
  uint32_t originate_timestamp_seconds = bswap_32(packet->originate_timestamp_seconds);
  uint32_t receive_timestamp_seconds = bswap_32(packet->receive_timestamp_seconds);
  uint64_t transmit_timestamp_seconds = bswap_32(packet->transmit_timestamp_seconds);
  uint32_t transmit_timestamp_fractions = bswap_32(packet->transmit_timestamp_fractions);

  fprintf(out, "Reference Timestamp : %u(%lu)\n", reference_timestamp_seconds, reference_timestamp_seconds - 2208988800L);
  fprintf(out, "Origin Timestamp : %u(%lu)\n", originate_timestamp_seconds, originate_timestamp_seconds - 2208988800L);
  fprintf(out, "Receive Timestamp : %u(%lu)\n", receive_timestamp_seconds, receive_timestamp_seconds - 2208988800L);
  fprintf(out, "Transmit Timestamp seconds: %lu(%lu)\n", transmit_timestamp_seconds, transmit_timestamp_seconds - 2208988800L);
  fprintf(out, "Transmit Timestamp fractions : %u\n", transmit_timestamp_fractions);
  fprintf(out, "Transmit Timestamp seconds: %f\n", (((transmit_timestamp_seconds - 2208988800L) << 32) + transmit_timestamp_fractions) / (0x1p+32));
  if (transmit_timestamp_seconds)
  {
    time_t machine_time = time(NULL);
    time_t ntp_time = ntohl(packet->transmit_timestamp_seconds) - 2208988800L;
    struct tm *lpNewLocalTime = localtime(&ntp_time);
    if (lpNewLocalTime == NULL)
    {
      perror("localtime");
      exit(EXIT_FAILURE);
    }
    char buf[32];
    memset(buf, 0, 32);
    fprintf(out, _("Local time : %s"), ctime_r(&machine_time, buf));
    fprintf(out, _("NTP server time : %s"), ctime_r(&ntp_time, buf));
  }
}
