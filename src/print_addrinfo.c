

#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include <stdio.h>

/**
 * アドレスとポート番号を表示する。
 * <I> adrinf: アドレス情報
 */
void print_addrinfo(struct addrinfo *adrinf)
{
	char hbuf[NI_MAXHOST];	/* 返されるアドレスを格納する */
	char sbuf[NI_MAXSERV];	/* 返されるポート番号を格納する */
	int rc;

	/* アドレス情報に対応するアドレスとポート番号を得る */
	rc = getnameinfo(adrinf->ai_addr, adrinf->ai_addrlen,
						hbuf, sizeof(hbuf),
						sbuf, sizeof(sbuf),
						NI_NUMERICHOST | NI_NUMERICSERV);
	if (rc != 0) {
		fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(rc));
		return;
	}

	printf("[%s]:%s\n", hbuf, sbuf);
}
