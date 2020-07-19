
#include "config.h"
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

#define SERVER_NAME "ntp.nict.jp"
#define SERVER_PORT "ntp"

/* _("NTP packet") */
struct NTP_Packet
{
  int Control_Word;
  int root_delay;
  int root_dispersion;
  int reference_identifier;
  int64_t reference_timestamp;
  int64_t originate_timestamp;
  int64_t receive_timestamp;
  int transmit_timestamp_seconds;
  int transmit_timestamp_fractions;
};
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
  /* [1024,65536) */
  unsigned short port_num = (unsigned short)(genrand64_real2() * 64512 + 1024);
  char port_str[8];
  snprintf(port_str, 8, "%d", port_num);
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  int err = getaddrinfo(NULL, port_str, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo localhost");
    return EXIT_FAILURE;
  }

  int recv_sock = 0;
  struct addrinfo *ptr;
  int sock_ai_family;
  int sock_ai_protocol;

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
  freeaddrinfo(res);

  printf("%ld\n", sizeof(struct NTP_Packet));
  struct NTP_Packet send;
  send.Control_Word = htonl(0x0B000000);
  send.root_delay = 0;
  send.root_dispersion = 0;
  send.reference_identifier = 0;
  send.reference_timestamp = 0;
  send.originate_timestamp = 0;
  send.receive_timestamp = 0;
  send.transmit_timestamp_seconds = 0;
  send.transmit_timestamp_fractions = 0;
  unsigned char *a = (unsigned char *)&send;
  int i = 0;
  for (; i < 48; i++)
  {
    printf("%02x", a[i]);
    if ((i % 16) == 15)
    {
      printf("\n");
    }
  }

  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  err = getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo");
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
  ssize_t w = sendto(recv_sock, &send, sizeof(struct NTP_Packet), 0, ptr->ai_addr, ptr->ai_addrlen);
  if (w == (ssize_t)-1)
  {
    perror("sendto");
    close(recv_sock);
    freeaddrinfo(res);
    return EXIT_FAILURE;
  }
  struct pollfd fds = {recv_sock, POLLIN, 0};
  struct timespec spec = {3, 0};
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
    close(recv_sock);
    return EXIT_FAILURE;
  }
  if ((fds.revents & POLLERR) != 0)
  {
    perror("isset failed");
    close(recv_sock);
    return EXIT_FAILURE;
  }
  struct NTP_Packet recv;
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

  time_t machine_time = time(NULL);
  time_t ntp_time = ntohl(recv.transmit_timestamp_seconds) - 2208988800L;
  struct tm *lpNewLocalTime = localtime(&ntp_time);
  if (lpNewLocalTime == NULL)
  {
    perror("localtime");
    return EXIT_FAILURE;
  }
  printf(_("Local time : %s"), ctime(&machine_time));
  printf(_("NTP server time : %s"), ctime(&ntp_time));
  return EXIT_SUCCESS;
}
