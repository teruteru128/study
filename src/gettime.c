
#include "gettime.h"

void ntp2tv(uint8_t ntp[8], struct timeval *tv)
{
  uint64_t aux = 0;
  uint8_t *p = ntp;
  int i;

  /* we get the ntp in network byte order, so we must
     * convert it to host byte order. */
  for (i = 0; i < 8 / 2; i++)
  {
    aux <<= 8;
    aux |= *p++;
  } /* for */

  /* now we have in aux the NTP seconds offset */
  aux -= OFFSET;
  tv->tv_sec = aux;

  /* let's go with the fraction of second */
  aux = 0;
  for (; i < 8; i++)
  {
    aux <<= 8;
    aux |= *p++;
  } /* for */

  /* now we have in aux the NTP fraction (0..2^32-1) */
  aux *= 1000000; /* multiply by 1e6 */
  aux >>= 32;     /* and divide by 2^32 */
  tv->tv_usec = aux;
} /* ntp2tv */

size_t print_tv(struct timeval *t)
{
  return (size_t)printf("%ld.%06ld\n", t->tv_sec, t->tv_usec);
}

size_t print_ts(struct timespec *t)
{
  return (size_t)printf("%ld.%09ld\n", t->tv_sec, t->tv_nsec);
}

size_t print_ntp(uint8_t ntp[8])
{
  int i;
  int res = 0;
  for (i = 0; i < 8; i++)
  {
    if (i == 8 / 2)
      res += printf(".");
    res += printf("%02x", ntp[i]);
  } /* for */
  res += printf("\n");
  return res;
} /* print_ntp */

void tv2ntp(struct timeval *tv, uint8_t ntp[8])
{
  uint64_t aux = 0;
  uint8_t *p = ntp + 8;
  int i;

  aux = tv->tv_usec;
  aux <<= 32;
  aux /= 1000000;

  /* we set the ntp in network byte order */
  for (i = 0; i < 8 / 2; i++)
  {
    *--p = aux & 0xff;
    aux >>= 8;
  } /* for */

  aux = tv->tv_sec;
  aux += OFFSET;

  /* let's go with the fraction of second */
  for (; i < 8; i++)
  {
    *--p = aux & 0xff;
    aux >>= 8;
  } /* for */

} /* ntp2tv */

int main(int argc, char **argv)
{

  int recv_sd = 0;
  struct addrinfo hints, *res, *ptr;
  SNTP sendsntp;
  SNTP recvsntp;
  struct timeval tv;
  struct timespec ts;
  uint8_t ntp[8];

  gettimeofday(&tv, NULL);
  clock_gettime(CLOCK_REALTIME, &ts);

  tv2ntp(&tv, ntp);
  printf("tv : ");
  print_tv(&tv);
  printf("ntp: ");
  print_ntp(ntp);

  memset(&sendsntp, 0, sizeof(SNTP));
  memset(&recvsntp, 0, sizeof(SNTP));

  sendsntp.li = 0;
  sendsntp.vn = 4;
  sendsntp.mode = 3;
  sendsntp.stratum = 0;
  sendsntp.poll = 0;
  sendsntp.precison = 0;
  sendsntp.root_delay = 0;
  sendsntp.root_dispresion = 0;
  sendsntp.reference_identifire = 0;

  printf("li : %d\n", sendsntp.li);
  printf("vn : %d\n", sendsntp.vn);
  printf("mode : %d\n", sendsntp.mode);
  printf("stratum : %d\n", sendsntp.stratum);
  printf("poll : %d\n", sendsntp.poll);
  printf("precison : %d\n", sendsntp.precison);
  printf("root delay : %d\n", sendsntp.root_delay);
  printf("root dispresion : %d\n", sendsntp.root_dispresion);
  printf("reference identifire : %d\n", sendsntp.reference_identifire);

  {
    unsigned char *tmp = (unsigned char *)&sendsntp;
    //tmp[0] = 0xb0;
    //memcpy(&(sntp.transmit_timestamp), ntp, 8);
    for (size_t i = 0; i < sizeof(SNTP); i++)
    {
      printf("%02x", (tmp[i] & 0xff));
      if (i % 16 == 15)
      {
        puts("");
      }
    }
  }

  hints.ai_flags = 0;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  int ret = getaddrinfo(DEFAULT_SERVER, "ntp", &hints, &res);
  if (ret != 0)
  {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
    return EXIT_FAILURE;
  }

  for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
  {
    recv_sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (recv_sd < 0)
      continue;
  }

  puts(DEFAULT_SERVER);

  if (sendto(recv_sd, &sendsntp, sizeof(SNTP), 0, ptr->ai_addr, ptr->ai_addrlen) < 0)
  {
    perror("sendto");
    return EXIT_FAILURE;
  }

  fputs("successfully send!\nbefore:\n", stdout);
  {
    unsigned char *tmp = (unsigned char *)&recvsntp;
    for (size_t i = 0; i < 48; i++)
    {
      printf("%02x", tmp[i]);
      if (i % 16 == 15)
      {
        fputs("\n", stdout);
      }
    }
  }

  if (recvfrom(recv_sd, &recvsntp, sizeof(recvsntp), 0, ptr->ai_addr, &ptr->ai_addrlen) < 0)
  {
    perror("recvfrom");
    return 1;
  }

  char str[NI_MAXHOST];
  if (inet_ntop(ptr->ai_family, ptr->ai_addr, str, NI_MAXHOST) != NULL)
  {
    printf("%s\n", str);
  }
  freeaddrinfo(res);

  puts("after:");
  {
    unsigned char *tmp = (unsigned char *)&recvsntp;
    for (size_t i = 0; i < 48; i++)
    {
      printf("%02x", (tmp[i] & 0xff));
      if (i % 16 == 15)
      {
        puts("");
      }
    }
  }
  printf("%d %d %d %d %d %d\n", recvsntp.li, recvsntp.vn, recvsntp.mode, recvsntp.stratum, recvsntp.poll, recvsntp.precison);

  if (recv_sd != 0)
  {
    close(recv_sd);
    recv_sd = 0;
  }
  return EXIT_SUCCESS;
}
