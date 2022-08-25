
#define _GNU_SOURCE

#include <inttypes.h>
#include <math.h>
#include <stdfix.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 */
int hiho(int argc, char **argv, const char **envp)
{
    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t b_max = 0;
    uint64_t c = 0x44088125286DL ^ 0x5DEECE66DL;
    b = ((c * 0x5DEECE66DL) + 0xb) & 0xFFFFFFFFFFFFL;
    a = ((b * 0x5DEECE66DL) + 0xb) & 0xFFFFFFFFFFFFL;
    printf("0x%016" PRIx64 "->0x%016" PRIx64 "->0x%016" PRIx64 "\n",
           c ^ 0x5DEECE66DL, b, a);
    printf("--\n");
    for (a = 0xffffff000000L; a < 0x1000000000000L; a++)
    {
        b = ((a - 0xb) * 0xDFE05BCB1365L) & 0xFFFFFFFFFFFFL;
        c = ((b - 0xb) * 0xDFE05BCB1365L) & 0xFFFFFFFFFFFFL;
        if ((b & 0xffffff000000L) == 0xffffff000000L)
        {
            printf("%1$" PRIu64 ", 0x%1$016" PRIx64 "->0x%2$016" PRIx64 "->0x%3$016" PRIx64 "\n",
                   c ^ 0x5DEECE66DL, b, a);
        }
    }
    printf("--\n");
    union a
    {
        double a;
        uint64_t b;
    } d;
    union b
    {
        float a;
        int32_t b;
    } e;

    d.a = 1;
    printf("%la, %016" PRIx64 "\n", d.a, d.b);
    d.b = 0x3fefffffffffffff;
    printf("%la, %016" PRIx64 "\n", d.a, d.b);
    e.a = 0xffffff / (float)(1 << 24);
    printf("%a, %08" PRIx32 "\n", e.a, e.b);
    e.b = 0x3f7fffff;
    printf("%1$a, %1$f, %2$08" PRIx32 "\n", e.a, e.b);
    e.b = 0x7fc00000;
    printf("%1$a, %1$f, %2$08" PRIx32 "\n", e.a, e.b);
    printf("--\n");
    printf("%lu\n", (0x5DEECE66DUL * 0xDFE05BCB1365UL) & 0xFFFFFFFFFFFFUL);
    printf("%012lx\n", (0x5DEECE66DUL * 0x5DEECE66DUL) & 0xFFFFFFFFFFFFUL);
    printf("%012lx\n", (0xbb20b4600a69UL * 0x5DEECE66DUL) & 0xFFFFFFFFFFFFUL);
    printf("%012lx\n", (0xe7a191a625d9L * 0xDFE05BCB1365UL) & 0xFFFFFFFFFFFFUL);
    printf("%lu\n", (0xd498bd0ac4b5UL * 0x13a1f16f099dUL) & 0xFFFFFFFFFFFFUL);
    a = 1;
    b = ((a * 0x5DEECE66DL) + 0xb) & 0xFFFFFFFFFFFFL;
    c = ((b * 0x5DEECE66DL) + 0xb) & 0xFFFFFFFFFFFFL;
    printf("%016lx\n", c);
    printf("%016lx\n", (c - 0x40942DE6BAUL) * 0xe7a191a625d9L & 0xFFFFFFFFFFFFL);
    printf("%016lx\n", ((1 * 0xbb20b4600a69UL) + 0x40942DE6BAUL) & 0xFFFFFFFFFFFFL);
    printf("%016lx\n", (0xe7a191a625d9L * 0xbb20b4600a69UL) & 0xffffffffffffL);
    return 0;
}
