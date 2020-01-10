
#include "gettime.h"

void ntp2tv(uint8_t ntp[8], struct timeval *tv)
{
    uint64_t aux = 0;
    uint8_t *p = ntp;
    int i;

    /* we get the ntp in network byte order, so we must
     * convert it to host byte order. */
    for (i = 0; i < 8/ 2; i++) {
        aux <<= 8;
        aux |= *p++;
    } /* for */

    /* now we have in aux the NTP seconds offset */
    aux -= OFFSET;
    tv->tv_sec = aux;

    /* let's go with the fraction of second */
    aux = 0;
    for (; i < 8; i++) {
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
    return printf("%ld.%06ld\n", t->tv_sec, t->tv_usec);
}

size_t print_ntp(uint8_t ntp[8])
{
    int i;
    int res = 0;
    for (i = 0; i < 8; i++) {
        if (i == 8/ 2)
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
    for (i = 0; i < 8/2; i++) {
        *--p = aux & 0xff;
        aux >>= 8;
    } /* for */

    aux = tv->tv_sec;
    aux += OFFSET;

    /* let's go with the fraction of second */
    for (; i < 8; i++) {
        *--p = aux & 0xff;
        aux >>= 8;
    } /* for */

} /* ntp2tv */

int main(int argc, char** argv){

	int serv_sd = 0;
	int recv_sd = 0;
	struct sockaddr_in addr;
	socklen_t sin_size;
	struct sockaddr_in recv_addr;
	struct sockaddr_in from_addr;
	SNTP sntp;
	char buf[2048];
	int i = 0;
	struct timeval t;
	uint8_t ntp[8];

	gettimeofday(&t, NULL);

	tv2ntp(&t, ntp);
	printf("tv : "); print_tv(&t);
	printf("ntp: "); print_ntp(ntp);

	memset(&sntp, 0, sizeof(SNTP));
	memset(buf, 0, sizeof(buf));

	sntp.li = 0;
	sntp.vn = 4;
	sntp.mode = 3;
	sntp.stratum = 0;
	sntp.poll = 0;
	sntp.precison = 0;
	sntp.root_delay = 0;
	sntp.root_dispresion = 0;
	sntp.reference_identifire = 0;
	char *tmp = (char*)&sntp;
//	tmp[0] = 0xb0;
	//memcpy(&(sntp.transmit_timestamp), ntp, 8);

	printf("li : %d\n", sntp.li);
	printf("vn : %d\n", sntp.vn);
	printf("mode : %d\n", sntp.mode);
	printf("stratum : %d\n", sntp.stratum);
	printf("poll : %d\n", sntp.poll);
	printf("precison : %d\n", sntp.precison);
	printf("root delay : %d\n", sntp.root_delay);
	printf("root dispresion : %d\n", sntp.root_dispresion);
	printf("reference identifire : %d\n", sntp.reference_identifire);

	for(i = 0; i < sizeof(SNTP); i++){
		printf("%02x", (tmp[i] & 0xff));
			if(i % 16 == 15){
			puts("");
		}
	}

	if((recv_sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("recv socket");
	}
	// 送信用ソケット作成
	if((serv_sd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		perror("serv socket");
		return 1;
	}

	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(123);
	recv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(recv_sd, (struct sockaddr*) &recv_addr, sizeof(recv_addr)) < 0){
		perror("bind");
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(123);
	// TODO: support ipv6
	addr.sin_addr.s_addr = inet_addr(DEFAULT_SERVER);
	puts(DEFAULT_SERVER);

	if(sendto(serv_sd, (const char *)&sntp, sizeof(SNTP), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		perror("sendto");
		return 1;
	}

	puts("successfully send!");
/*
	close(serv_sd);

	serv_sd = 0;
*/
	puts("before:");
	for(i = 0; i < 64; i++){
		printf("%02x", buf[i]);
		if(i % 16 == 15){
			puts("");
		}
	}

	if(recvfrom(serv_sd, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &sin_size) < 0) {
		perror("recvfrom");
		return 1;
	}

	char str[4096];
	if(inet_ntop(AF_INET, &addr, str, sizeof(addr)) != NULL){
		printf("%s\n", str);
	}

	puts("after:");
	for(i = 0; i < 64; i++){
		printf("%02x", (buf[i] & 0xff));
		if(i % 16 == 15){
			puts("");
		}
	}

	if(serv_sd != 0){
		close(serv_sd);
		serv_sd = 0;
	}
	if(recv_sd != 0){
		close(recv_sd);
		recv_sd = 0;
	}
	printf("%08x\n", htonl(0x0B000000));
	return 0;
}

