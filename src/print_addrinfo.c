
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include <stdio.h>
#include "print_addrinfo.h"

/**
 * アドレスとポート番号を表示する。
 * <I> adrinf: アドレス情報
 */
void print_addrinfo0(struct addrinfo *adrinf, FILE *stream)
{
  char hbuf[NI_MAXHOST]; /* 返されるアドレスを格納する */
  char sbuf[NI_MAXSERV]; /* 返されるポート番号を格納する */
  int rc;

  /* アドレス情報に対応するアドレスとポート番号を得る */
  rc = getnameinfo(adrinf->ai_addr, adrinf->ai_addrlen,
                   hbuf, sizeof(hbuf),
                   sbuf, sizeof(sbuf),
                   NI_NUMERICHOST | NI_NUMERICSERV);
  if (rc != 0)
  {
    perror("getnameinfo");
    return;
  }

  fprintf(stream, "[%s]:%s\n", hbuf, sbuf);
}

void print_addrinfo(struct addrinfo *adrinf)
{
  print_addrinfo0(adrinf, stdout);
}
