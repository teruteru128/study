
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

  int recv_sock = 0;
  int sock_ai_family = 0;
  int sock_ai_protocol = 0;

  /*
    * ローカルポート選定
    * [1024,65536)
    */
  unsigned short port_num = 0;
  char port_str[8];
  struct addrinfo hints, *res = NULL, *ptr = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_socktype = SOCK_DGRAM;
  //hints.ai_family = AF_INET6;
  hints.ai_flags = AI_PASSIVE;
  int err = 0;
  port_num = (unsigned short)(genrand64_real2() * 64512 + 1024);
  snprintf(port_str, 8, "%d", port_num);
  err = getaddrinfo(NULL, port_str, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo localhost");
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
  printf("protocol : %d\n", ptr->ai_protocol);
  print_addrinfo(ptr);
  freeaddrinfo(res);
  res = NULL;
  ptr = NULL;

  struct NTP_Packet send;
  memset(&send, 0, sizeof(struct NTP_Packet));
  send.Control_Word = htonl(0x0B000000);
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
