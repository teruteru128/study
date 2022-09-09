
#define _GNU_SOURCE
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <errno.h>
#include <gmp.h>
#include <math.h>
#include <netdb.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

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

    if (argc < 3)
    {
        fprintf(stderr, "initialvalueファイル2個入れてクレメンス\n");
        fprintf(stderr, "initial value っていうかbase numberですよねこれ\n");
        return 1;
    }
    FILE *in1 = fopen(argv[1], "r");
    FILE *in2 = fopen(argv[2], "r");
    if (in1 == NULL || in2 == NULL)
    {
        if (in1 != NULL)
        {
            fclose(in1);
        }
        if (in2 != NULL)
        {
            fclose(in2);
        }
        return 1;
    }
    __mpz_struct p[3];
    mpz_inits(p, p + 1, p + 2, NULL);
    mpz_inp_str(p, in1, 16);
    mpz_inp_str(p + 1, in2, 16);
    fclose(in1);
    fclose(in2);
    mpz_mul(p + 2, p, p + 1);
    size_t size = mpz_size(p + 2);
    size_t sizeinbase2 = mpz_sizeinbase(p + 2, 2);
    printf("%zu, %zu\n", size, sizeinbase2);
    mpz_clears(p, p + 1, p + 2, NULL);
    return 0;
}
