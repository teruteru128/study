/*
 * cl.c - �����Ф���α�������������Υ��饤�����
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

/**
 * ���ɥ쥹�ȥݡ����ֹ��ɽ�����롣
 * <I> adrinf: ���ɥ쥹����
 */
void print_addrinfo(struct addrinfo *adrinf)
{
	char hbuf[NI_MAXHOST];	/* �֤���륢�ɥ쥹���Ǽ���� */
	char sbuf[NI_MAXSERV];	/* �֤����ݡ����ֹ���Ǽ���� */
	int rc;

	/* ���ɥ쥹������б����륢�ɥ쥹�ȥݡ����ֹ������ */
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

/**
 * �������ȥ��å�
 */
int main(int argc, char *argv[])
{
	struct addrinfo hints;		/* �������������ɥ쥹�����ؼ����� */
	struct addrinfo *res;		/* �����������ɥ쥹�����֤äƤ��� */
	struct addrinfo *adrinf;	/* ��³�׵���˻Ȥ� */

	int rc;

	/* �����ο�������å����� */
	if (argc != 3) {
		fprintf(stderr, "usage: %s nodename servname\n", argv[0]);
		return -1;
	}

	/* �����ǻ��ꤵ�줿���ɥ쥹���ݡ����ֹ椫�饢�ɥ쥹��������� */
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (rc != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
		return -1;
	}

	/* ����줿���ɥ쥹���٤Ƥ��Ф���³��Ԥ� */
	for (adrinf = res; adrinf != NULL; adrinf = adrinf->ai_next) {

		char buf[2048];
		int sock;
		int len;

		/*
		 * ��³�׵�򤹤롣
		 */
		sock = socket(adrinf->ai_family, adrinf->ai_socktype, adrinf->ai_protocol);
		if (sock < 0) {
			perror("socket()");
			continue;
		}

		rc = connect(sock, adrinf->ai_addr, adrinf->ai_addrlen);
		if (rc < 0) {
			perror("connect()");
			close(sock);
			continue;
		}

		/* ���ɥ쥹�����ɽ������ */
		print_addrinfo(adrinf);

		/* �����Ф���α�����ɽ������ */
		while (0 < (len = read(sock, buf, sizeof(buf)))) {
			printf("%.*s", len, buf);
		}

		close(sock);
	}

	return 0;
}

