
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()
#include <errno.h>
#include <time.h>
#include <random.h>
#include <print_addrinfo.h>
#include <poll.h>
#include <java_random.h>
#include <math.h>
#include <ctype.h>
#include <err.h>
#include <byteswap.h>

#define SERVER_NAME "ntp.nict.jp"
#define SERVER_PORT "ntp"

/* _("NTP packet") */
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

static void dumpNTPpacket(struct NTP_Packet *packet)
{
  unsigned char *a = (unsigned char *)packet;
  int i = 0;
  for (; i < 48; i++)
  {
    printf("%02x", a[i]);
    if ((i % 16) == 15)
    {
      printf("\n");
    }
  }

  uint32_t w = bswap_32(packet->Control_Word);
  int leap_indicator = (w >> 30) & 0x03;
  printf("LI : %d\n", leap_indicator);
  int version_number = (w >> 27) & 0x07;
  printf("VN : %d\n", version_number);
  int mode = (w >> 24) & 0x07;
  printf("mode : %d\n", mode);
  int stratum = (w >> 16) & 0xff;
  printf("Stratum : %d\n", stratum);
  char poll = (w >> 8) & 0xff;
  printf("Poll : %d(%d)\n", poll, 1 << poll);
  char pre = (char)(w >> 0) & 0xff;
  printf("Precision : %d(%f)\n", pre, pow(2, pre));
  int root_delay = bswap_32(packet->root_delay);
  printf("Root Delay : %d(%f)\n", root_delay, root_delay / 0x1p+16);
  int root_dispersion = bswap_32(packet->root_dispersion);
  printf("Root Dispersion : %d(%f)\n", root_dispersion, root_dispersion / 0x1p+16);
  int reference_identifier = packet->reference_identifier;
  printf("Reference ID : %08x", reference_identifier);
  char refid[5];
  memset(refid, 0, sizeof(refid));
  memcpy(refid, &reference_identifier, 4);
  if (stratum == 1)
    printf("(%s)", refid);
  if (version_number == 3 && stratum >= 2)
  {
    struct in_addr ad = {htonl(reference_identifier)};
    char *addrstr = inet_ntoa(ad);
    if(addrstr)
      printf("(%s)", addrstr);
  }
  if (version_number == 4 && stratum >= 2)
  {
    printf("(*´ω｀*)");
  }
  printf("\n");
  uint32_t reference_timestamp_seconds = bswap_32(packet->reference_timestamp_seconds);
  uint32_t originate_timestamp_seconds = bswap_32(packet->originate_timestamp_seconds);
  uint32_t receive_timestamp_seconds = bswap_32(packet->receive_timestamp_seconds);
  uint64_t transmit_timestamp_seconds = bswap_32(packet->transmit_timestamp_seconds);
  uint32_t transmit_timestamp_fractions = bswap_32(packet->transmit_timestamp_fractions);

  printf("Reference Timestamp : %u(%lu)\n", reference_timestamp_seconds, reference_timestamp_seconds - 2208988800L);
  printf("Origin Timestamp : %u(%lu)\n", originate_timestamp_seconds, originate_timestamp_seconds - 2208988800L);
  printf("Receive Timestamp : %u(%lu)\n", receive_timestamp_seconds, receive_timestamp_seconds - 2208988800L);
  printf("Transmit Timestamp seconds: %lu(%lu)\n", transmit_timestamp_seconds, transmit_timestamp_seconds - 2208988800L);
  printf("Transmit Timestamp fractions : %u\n", transmit_timestamp_fractions);
  printf("Transmit Timestamp seconds: %f\n", (((transmit_timestamp_seconds - 2208988800L) << 32) + transmit_timestamp_fractions) / (0x1p+32));
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
    printf(_("Local time : %s"), ctime(&machine_time));
    printf(_("NTP server time : %s"), ctime(&ntp_time));
  }
}

/*
https://www.nakka.com/lib/inet/sntpc.html
https://www.nakka.com/lib/inet/sntpcex.html
*/
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  uint64_t seed[312];
  read_random(seed, sizeof(uint64_t), 312, 0);
  init_by_array64(seed, 312);
  int64_t s = 0;
  {
    union f
    {
      int64_t s;
      char buf[8];
    };
    union f f;
    read_random(&f.buf[2], sizeof(char), 6, 0);
    s = f.s;
  }

  int recv_sock = 0;
  int sock_ai_family = 0;
  int sock_ai_protocol = 0;

  /*
    * ローカルポート選定
    * [1024,65536)
    */
  //unsigned short port_num = (unsigned short)(genrand64_real2() * 64512 + 1024);
  unsigned short port_num = (unsigned short)(nextDouble(&s) * 64512 + 1024);
  char port_str[8];
  snprintf(port_str, 8, "%d", port_num);

  struct addrinfo hints, *res = NULL, *ptr = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_DGRAM;
  //hints.ai_family = AF_INET6;
  hints.ai_flags = AI_PASSIVE;

  int err = getaddrinfo(NULL, port_str, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo localhost");
    fprintf(stderr, "%s\n", gai_strerror(err));
    return EXIT_FAILURE;
  }

  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    recv_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (recv_sock < 0)
    {
      perror("socket recv_sock");
      freeaddrinfo(res);
      return EXIT_FAILURE;
    }
    err = bind(recv_sock, ptr->ai_addr, res->ai_addrlen);
    if (err < 0)
    {
      close(recv_sock);
      perror("bind");
      continue;
    }
    sock_ai_family = ptr->ai_family;
    sock_ai_protocol = ptr->ai_protocol;
    break;
  }
  if (ptr == NULL)
  {
    perror("bind failed");
    close(recv_sock);
    return EXIT_FAILURE;
  }
  print_addrinfo0(ptr, stderr);
  printf("%d\n", ptr->ai_socktype);
  printf("%d\n", ptr->ai_protocol);
  freeaddrinfo(res);
  res = NULL;
  ptr = NULL;

  struct NTP_Packet send;
  memset(&send, 0, sizeof(struct NTP_Packet));
  send.Control_Word = htonl(0x23000000);
  dumpNTPpacket(&send);

  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  err = getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo ntp");
    fprintf(stderr, "%s\n", gai_strerror(err));
    freeaddrinfo(res);
    close(recv_sock);
    return EXIT_FAILURE;
  }

  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    if (ptr->ai_family == sock_ai_family && ptr->ai_protocol == sock_ai_protocol)
    {
      break;
    }
  }
  if (ptr == NULL)
  {
    perror("host not found");
    freeaddrinfo(res);
    close(recv_sock);
    return EXIT_FAILURE;
  }
  print_addrinfo(ptr);
  ssize_t w = sendto(recv_sock, &send, sizeof(struct NTP_Packet), 0, ptr->ai_addr, ptr->ai_addrlen);
  if (w == (ssize_t)-1)
  {
    perror("sendto");
    close(recv_sock);
    freeaddrinfo(res);
    return EXIT_FAILURE;
  }
  struct pollfd fds = {recv_sock, POLLIN, 0};
  struct timespec spec = {10, 0};
  sigset_t sigmask = {0};

  int selret = ppoll(&fds, 1, &spec, &sigmask);
  printf("selret : %d\n", selret);
  if (selret == -1)
  {
    perror("select");
    close(recv_sock);
    return EXIT_FAILURE;
  }
  if (selret == 0)
  {
    perror("select timeout");
    //close(recv_sock);
  }
  if ((fds.revents & POLLERR) != 0)
  {
    perror("isset failed");
    close(recv_sock);
    return EXIT_FAILURE;
  }
  struct NTP_Packet recv;
  memset(&recv, 0, sizeof(struct NTP_Packet));
  w = recvfrom(recv_sock, &recv, sizeof(struct NTP_Packet), 0, res->ai_addr, &res->ai_addrlen);
  if (w == (ssize_t)-1)
  {
    perror("recv failed");
    close(recv_sock);
    freeaddrinfo(res);
    return EXIT_FAILURE;
  }
  freeaddrinfo(res);
  close(recv_sock);
  dumpNTPpacket(&recv);
  return EXIT_SUCCESS;
}
