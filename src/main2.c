
#define _GNU_SOURCE

#include <inttypes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/types.h>
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
 */
int hiho(int argc, char **argv, const char **envp)
{
    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t b_max = 0;
    uint64_t c = 0x44088125286DL ^ 0x5DEECE66DL;
    b = ((c * 0x5DEECE66DL) + 0xb) & 0xFFFFFFFFFFFFL;
    a = ((b * 0x5DEECE66DL) + 0xb) & 0xFFFFFFFFFFFFL;
    printf("0x%016" PRIx64 "->0x%016" PRIx64 "->0x%016" PRIx64 "\n", c ^ 0x5DEECE66DL, b, a);
    a = 0xffffff000000L;
    for (; a < 0x1000000000000L; a++)
    {
        b = ((a - 0xb) * 0xDFE05BCB1365L) & 0xFFFFFFFFFFFFL;
        c = ((b - 0xb) * 0xDFE05BCB1365L) & 0xFFFFFFFFFFFFL;
        if ((b & 0xffffff800000L) == 0xffffff800000L)
        {
            printf("0x%016" PRIx64 "->0x%016" PRIx64 "->0x%016" PRIx64 "\n", c ^ 0x5DEECE66DL, b, a);
            b_max = b;
        }
    }
    // ↓2回連続getFloatで-1が出るseed 2つ
    // 125352706827826
    // 116229385253865
    return 0;
}
