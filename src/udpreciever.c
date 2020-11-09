
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int main()
{
  struct addrinfo hints, *res = NULL, *ptr = NULL;
  hints.ai_flags = 0;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  int rc = getaddrinfo("0.0.0.0", "12345", &hints, &res);
  if (rc != 0)
  {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
    return EXIT_FAILURE;
  }

  int sock = -1;
  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (sock != -1)
    {
      break;
    }
  }

  if (sock == -1)
  {
    perror("socket");
    return EXIT_FAILURE;
  }

  int r = bind(sock, ptr->ai_addr, ptr->ai_addrlen);

  if (r == -1)
  {
    perror("bind");
    return EXIT_FAILURE;
  }

  char buf[BUFSIZ];
  do
  {
    memset(buf, 0, BUFSIZ);
    ssize_t len = recv(sock, buf, BUFSIZ, 0);

    printf("%s : %zd\n", buf, len);
  } while (strcmp(buf, "end") != 0);
  r = close(sock);
  freeaddrinfo(res);

  if (r)
    perror("close");

  return EXIT_SUCCESS;
}
