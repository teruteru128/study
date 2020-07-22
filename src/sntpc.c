
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
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strcmp()
#include <unistd.h> //close()
#include <print_addrinfo.h>
#include <poll.h>
#include <byteswap.h>
#include <signal.h>
#include "ntp.h"

#define SERVER_NAME "ntp.nict.jp"
#define SERVER_PORT "ntp"
/*
https://www.nakka.com/lib/inet/sntpc.html
https://www.nakka.com/lib/inet/sntpcex.html
*/
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);

  /* ソケット(ディスクリプタ) */
  int recv_sock = 0;

  /* NTPサーバのアドレス情報 */
  struct addrinfo hints, *res = NULL, *ptr = NULL;
  //memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = 0;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  int err = getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &res);
  if (err != 0)
  {
    perror("getaddrinfo localhost");
    fprintf(stderr, "%s\n", gai_strerror(err));
    return EXIT_FAILURE;
  }

  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    print_addrinfo0(ptr, stderr);
  }

  fputs("--\n", stderr);

  /* ソケットを作成して接続 */
  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    recv_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (recv_sock == -1)
      continue;

    if (connect(recv_sock, ptr->ai_addr, ptr->ai_addrlen) != -1)
      break;

    close(recv_sock);
  }
  if (ptr == NULL)
  {
    perror("connect failed");
    close(recv_sock);
    return EXIT_FAILURE;
  }
  print_addrinfo0(ptr, stderr);
  freeaddrinfo(res);
  res = NULL;
  ptr = NULL;

  struct NTP_Packet send;
  memset(&send, 0, sizeof(struct NTP_Packet));
  send.Control_Word = bswap_32(0x23000000);
  dumpNTPpacket(&send, stderr);

  ssize_t w = write(recv_sock, &send, sizeof(struct NTP_Packet));
  if (w == (ssize_t)-1)
  {
    perror("write");
    close(recv_sock);
    freeaddrinfo(res);
    return EXIT_FAILURE;
  }
  struct pollfd fds = {recv_sock, POLLIN, 0};
  struct timespec spec = {10, 0};
  sigset_t sigmask;
  sigfillset(&sigmask);

  int selret = ppoll(&fds, 1, &spec, &sigmask);
  fprintf(stderr, "selret : %d\n", selret);
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
  fprintf(stderr, "revents : %04x\n", fds.revents);
  struct NTP_Packet recv;
  memset(&recv, 0, sizeof(struct NTP_Packet));
  w = read(recv_sock, &recv, sizeof(struct NTP_Packet));
  if (w == (ssize_t)-1)
  {
    perror("read");
    close(recv_sock);
    freeaddrinfo(res);
    return EXIT_FAILURE;
  }
  freeaddrinfo(res);
  close(recv_sock);
  dumpNTPpacket(&recv, stderr);
  return EXIT_SUCCESS;
}
