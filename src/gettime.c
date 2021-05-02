
#include "gettime.h"

void ntp2tv(uint32_t ntp[2], struct timeval *tv)
{
    tv->tv_sec = ntohl(ntp[0]) - OFFSET;
    tv->tv_usec = (ntohl(ntp[1]) * 1000000L) >> 32;
} /* ntp2tv */

void ntp2ts(uint32_t ntp[2], struct timespec *ts)
{
    ts->tv_sec = ntohl(ntp[0]) - OFFSET;
    ts->tv_nsec = (ntohl(ntp[1]) * 1000000000L) >> 32;
}

size_t print_tv(struct timeval *t)
{
    return (size_t)printf("%ld.%06ld\n", t->tv_sec, t->tv_usec);
}

size_t print_ts(struct timespec *t)
{
    return (size_t)printf("%ld.%09ld\n", t->tv_sec, t->tv_nsec);
}

size_t print_ntp(uint32_t ntp[2])
{
    return (size_t)printf("%08x.%08x\n", ntohl(ntp[0]), ntohl(ntp[1]));
} /* print_ntp */

void tv2ntp(struct timeval *tv, uint32_t ntp[2])
{
    ntp[0] = htonl((uint32_t)(tv->tv_sec + OFFSET));
    ntp[1] = htonl((uint32_t)((tv->tv_usec << 32) / 1000000L));
} /* ntp2tv */

void ts2ntp(struct timespec *ts, uint32_t ntp[2])
{
    ntp[0] = htonl((uint32_t)(ts->tv_sec + OFFSET));
    ntp[1] = htonl((uint32_t)((ts->tv_nsec << 32) / 1000000000));
} /* ts2ntp */

int main(int argc, char **argv)
{

    int recv_sd = 0;
    struct addrinfo hints, *res, *ptr;
    SNTP sendsntp;
    SNTP recvsntp;
    struct timeval tv;
    struct timespec ts;
    uint32_t ntp[2];

    gettimeofday(&tv, NULL);
    clock_gettime(CLOCK_REALTIME, &ts);

    tv2ntp(&tv, ntp);
    printf("tv : ");
    print_tv(&tv);
    printf("ntp: ");
    print_ntp(ntp);

    ts2ntp(&ts, ntp);
    printf("ts : ");
    print_ts(&ts);
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
                fputs("\n", stdout);
            }
        }
    }

    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
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
        if (recv_sd >= 0)
            break;
    }

    if (ptr == NULL)
    {
        fputs("connected failed\n", stderr);
        freeaddrinfo(res);
        close(recv_sd);
        return EXIT_FAILURE;
    }

    puts(DEFAULT_SERVER);

    fputs("sending...\n", stderr);
    if (sendto(recv_sd, &sendsntp, sizeof(SNTP), 0, ptr->ai_addr, ptr->ai_addrlen) < 0)
    {
        perror("sendto");
        return EXIT_FAILURE;
    }
    fputs("sent!\n", stderr);

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

    fputs("receiving...\n", stderr);
    if (recvfrom(recv_sd, &recvsntp, sizeof(recvsntp), 0, ptr->ai_addr, &ptr->ai_addrlen) < 0)
    {
        perror("recvfrom");
        return 1;
    }
    fputs("received!\n", stderr);
    printf("li : %d\n", recvsntp.li);
    printf("vn : %d\n", recvsntp.vn);
    printf("mode : %d\n", recvsntp.mode);
    printf("stratum : %d\n", recvsntp.stratum);
    printf("poll : %d\n", recvsntp.poll);
    printf("precison : %d\n", recvsntp.precison);
    printf("root delay : %d\n", recvsntp.root_delay);
    printf("root dispresion : %d\n", recvsntp.root_dispresion);
    printf("reference identifire : %08x\n", recvsntp.reference_identifire);

    char addrtype[16] = "";
    char addrstr[NI_MAXHOST];
    char servstr[NI_MAXSERV];
    if (ptr->ai_family == AF_INET)
    {
        strncat(addrtype, "INET", 16);
    }
    else if (ptr->ai_family == AF_INET6)
    {
        strncat(addrtype, "INET6", 16);
    }
    else
    {
        snprintf(addrtype, 16, "OTHERS, %d", ptr->ai_family);
    }
    if (getnameinfo(ptr->ai_addr, ptr->ai_addrlen, addrstr, NI_MAXHOST, servstr, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
        perror("getnameinfo");
        freeaddrinfo(res);
        close(recv_sd);
        return EXIT_FAILURE;
    }
    printf("ip address : %s(%s)\n", addrstr, addrtype);
    freeaddrinfo(res);

    puts("after:");
    {
        unsigned char *tmp = (unsigned char *)&recvsntp;
        for (size_t i = 0; i < 48; i++)
        {
            printf("%02x", (tmp[i] & 0xff));
            if (i % 16 == 15)
            {
                fputs("\n", stdout);
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
