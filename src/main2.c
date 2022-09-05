
#define _GNU_SOURCE
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <openssl/bn.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int createlistensocket(int *family, int *protocol)
{
    int listen_socket = -1;
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    // service を "0" にしてlistenポートを開くテスト
    int rc = getaddrinfo(NULL, "0", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return -1;
    }
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((listen_socket
             = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol))
            == -1)
        {
            fprintf(stderr, "socket: %s\n", strerror(errno));
            continue;
        }
        printf("family: %d, socktype: %d, protocol: %d\n", ptr->ai_family,
               ptr->ai_socktype, ptr->ai_protocol);
        if (bind(listen_socket, ptr->ai_addr, ptr->ai_addrlen) < 0)
        {
            fprintf(stderr, "bind, %d: %s\n", listen_socket, strerror(errno));
            close(listen_socket);
            listen_socket = -1;
            continue;
        }
        if (listen(listen_socket, SOMAXCONN) < 0)
        {
            fprintf(stderr, "listen\n");
            close(listen_socket);
            listen_socket = -1;
            continue;
        }

        if (family != NULL)
        {
            *family = ptr->ai_family;
        }
        if (protocol != NULL)
        {
            *protocol = ptr->ai_protocol;
        }
        break;
    }
    freeaddrinfo(res);
    return listen_socket;
}

BIGNUM *a(BIGNUM *src)
{
    char *dec = BN_bn2dec(src);
    BIGNUM *desc = NULL;
    BN_hex2bn(&desc, dec);
    OPENSSL_free(dec);
    return desc;
}

/*
 * 秘密鍵かな？
 * ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc=
 * cm2E2vmE0Nd8aVP/4Ph2S1R6C5bkC1H7CiUBzbcDG3U=
 * BixgbLYk35GP+XHYdK/DgSWIUXyCTwCwEtY4h/G22dw=
 * BH4RDmdo0Yq0Ftiw0lm9ej5BmpZ35kEw2kaWZlZ0Do8=
 * lMhxDh6RPpWOsnJMeS12pTJ/j7EPn+ugpdbNQCGbiwc=
 * 9hZn+KDlwjgrbyFpaX5ibuaO4QfaFbIL79NUrwJlcRQ=
 * T+tDF4I700WFkFhGieYxWgQKPO/MDcntDYzMrqQSZjzwV2DzaI1OM/CsJWE30WBqMI1SxbEQHufR1A76I7ayWN==
 * nySkaCQxGErccmiqLznSQduXgFICpjnl2bo7n3FAhQMlku79plIeL85/etpN865GAnlUpErSppEYHvn4couGh3==
 * ns2bQQ4zlnfcCTSAxEH3gDDYHcBswKw92jQeEgm+9tse74XdX+LNwgfw7OsMUjOGtLMb7R/kXNRXYv1AHi71iV==
 * NxhJ5JwWhUtUccCfJNtVqzdpCMGOaAtknmcEKLyglZFNXE66EiFi9wPFekwekx3ln8m9v5wnfv7V8jSrpZ/SHQ==
 * +3n5qDbtpicXBy+Yyol/TJkg2IoQ01vZ/U2SvgpP+Fdm4DrIYngY7X0ZS53rc/KKIHT//jVqNwNBz1sGFyYUDg==
 * cLtHGFI7X/Xl6Ly03DczMzl2bsHJmI2BMQKKCckUek5vTIiltDPfT3PxdT6zxW1LzwVqJIsQEkxxPNTswgpSFg==
 * pMQBNF+F12AXT3T0mQq7S0l1VcCr/Dw2Q54zeuHH0/1ExLgbhHEsmAHf3WR9nK/Ku1Mc/eU3vaAO78yplJB76A==
 * QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQQ==
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 */
int hiho(int argc, char **argv, const char **envp)
{
    struct tm tm1 = { 0 };
    struct tm tm2 = { 0 };
    tm1.tm_year = tm2.tm_year = 2022 - 1900;
    tm1.tm_mon = tm2.tm_mon = 9 - 1;
    tm1.tm_mday = tm2.tm_mday = 5;
    tm1.tm_hour = tm2.tm_hour = 14;
    tm1.tm_min = 49;
    tm1.tm_sec = 57;

    tm2.tm_min = 52;
    tm2.tm_sec = 42;

    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);
    printf("%" PRId64 "\n", (int64_t)difftime(time2, time1));
    return 0;
}
