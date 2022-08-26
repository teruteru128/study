
#define _GNU_SOURCE

#include <inttypes.h>
#include <math.h>
#include <stdfix.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MULTIPLIER 0x5DEECE66DUL
#define ADDEND 0xBUL
#define INVERSE_MULTIPLIER 0xDFE05BCB1365UL

#define DOUBLE_MULTIPLIER 0xBB20B4600A69UL
#define DOUBLE_ADDEND 0x0040942DE6BAUL
#define DOUBLE_INVERSE_MULTIPLIER 0xE7A191A625D9UL

#define TRIPLE_MULTIPLIER 0xD498BD0AC4B5UL
#define TRIPLE_ADDEND 0x0AA8544E593DUL
#define TRIPLE_INVERSE_MULTIPLIER 0x13A1F16F099DUL

#define MASK 0xFFFFFFFFFFFFUL

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
    uint64_t c = 0x44088125286DL ^ MULTIPLIER;
    b = ((c * MULTIPLIER) + ADDEND) & 0xFFFFFFFFFFFFL;
    a = ((b * MULTIPLIER) + ADDEND) & 0xFFFFFFFFFFFFL;
    printf("0x%016" PRIx64 "->0x%016" PRIx64 "->0x%016" PRIx64 "\n",
           c ^ MULTIPLIER, b, a);
    printf("--\n");
    for (a = 0xffffff000000L; a < 0x1000000000000L; a++)
    {
        b = ((a - ADDEND) * 0xDFE05BCB1365L) & 0xFFFFFFFFFFFFL;
        c = ((b - ADDEND) * 0xDFE05BCB1365L) & 0xFFFFFFFFFFFFL;
        if ((b & 0xffffff000000L) == 0xffffff000000L)
        {
            printf("%1$" PRIu64 ", 0x%1$016" PRIx64 "->0x%2$016" PRIx64 "->0x%3$016" PRIx64 "\n",
                   c ^ MULTIPLIER, b, a);
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
    printf("%lu\n", (MULTIPLIER * INVERSE_MULTIPLIER) & MASK);
    printf("double multi: %012lx\n", (MULTIPLIER * MULTIPLIER) & MASK);
    printf("triple multi: %012lx\n", (DOUBLE_MULTIPLIER * MULTIPLIER) & MASK);
    printf("%012lx\n", (ADDEND * MULTIPLIER + ADDEND) & MASK);
    printf("triple inver: %012lx\n", (DOUBLE_INVERSE_MULTIPLIER * INVERSE_MULTIPLIER) & MASK);
    printf("%012lx\n", (TRIPLE_MULTIPLIER * TRIPLE_INVERSE_MULTIPLIER) & MASK);
    a = 1;
    b = ((a * MULTIPLIER) + ADDEND) & MASK;
    c = ((b * MULTIPLIER) + ADDEND) & MASK;
    printf("%012lx\n", c);
    printf("%012lx\n", (c - DOUBLE_ADDEND) * DOUBLE_INVERSE_MULTIPLIER & MASK);
    printf("%012lx\n", ((1 * DOUBLE_MULTIPLIER) + DOUBLE_ADDEND) & MASK);
    printf("%012lx\n", (DOUBLE_INVERSE_MULTIPLIER * DOUBLE_MULTIPLIER) & MASK);
    return 0;
}
