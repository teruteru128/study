
#define _GNU_SOURCE
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/provider.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD *md = EVP_MD_fetch(NULL, "sha512", NULL);
    if (md == NULL)
    {
        fprintf(stderr, "%s\n", ERR_reason_error_string(ERR_get_error()));
        return 1;
    }
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    size_t i = 0;
    for (uint64_t j = 0; j < 10; j++)
    {
        if (!EVP_DigestInit_ex2(ctx, md, NULL))
        {
            fprintf(stderr, "%s\n", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        if (!EVP_DigestUpdate(ctx, &j, 8))
        {
            fprintf(stderr, "%s\n", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        if (!EVP_DigestFinal_ex(ctx, hash, &length))
        {
            fprintf(stderr, "%s\n", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        printf("length:%d->%d\n", EVP_MAX_MD_SIZE, length);
        for (i = 0; i < length; i++)
        {
            /* code */
            printf("%02x", hash[i]);
        }
        printf("\n");
        EVP_MD_CTX_reset(ctx);
    }
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(md);
#endif
    return 0;
}
