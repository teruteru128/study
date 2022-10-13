
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "pngheaders.h"
#include <CL/opencl.h>
#include <errno.h>
#include <gmp.h>
#include <inttypes.h>
#include <java_random.h>
#include <math.h>
#include <netdb.h>
#include <omp.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <jpeglib.h> // jpeglibはstdioより下(FILEが依存しているため)

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/provider.h>
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
 * D8BH6DLNJekZ5jiiIVSnyS5ziE9XJSRG5bA9OdiFdjee6HTxHxFQXyEQdhfN+E69RKToLYXGDxK2X9v9eEcbUxdSp9tbptXegxkNQgIxg97BAq9gtmxPm4Ebngl/Q/I4
 * cLJlMSoCYBgR0d/bg7zG1B77BBWy7f1KLiJG5b8mPmlD8dAJKCZSEFRdWLuxSyRjgFFeiMm4+l+2SNIhL/SBma7ABhg232DeJkbUcZJKqBfAI9taPQ5Y9bwIXrcjxqMx
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * ↓getDoubleで可能な限り1に近い値が出るseed
 * 155239116123415
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 * ハッシュの各バイトを１バイトにORで集約して結果が0xffにならなかったら成功
 * 丸数字の1から50までforで出す
 */
int hiho(int argc, char **argv, const char **envp)
{

    struct IHDR ihdr = { 0 };
    printf("/mnt/g/iandm/image/pixiv.net/058/58755930_p0.png\n");
    png_byte **image1 = NULL;
    int ret = read_png("/mnt/g/iandm/image/pixiv.net/058/58755930_p0.png",
                       &ihdr, NULL, NULL, 0, &image1);
    png_byte **image2 = NULL;
    ret = read_png("/mnt/g/iandm/image/pixiv.net/058/58755930_p2.png", &ihdr,
                   NULL, NULL, 0, &image2);
    size_t x = 0;
    size_t y = 0;
    png_byte **diff1 = malloc(sizeof(png_byte *) * 1100);
    png_byte **diff2 = malloc(sizeof(png_byte *) * 1100);
    size_t offset = 0;
    for (y = 0; y < 1100; y++)
    {
        diff1[y] = malloc(sizeof(png_byte) * 815 * 4);
        diff2[y] = malloc(sizeof(png_byte) * 815 * 4);
        for (x = 0, offset = 0; x < 815; x++, offset += 4)
        {
            if (memcmp(image1[y] + offset, image2[y] + offset, 4) == 0)
            {
                // memset(diff1[y] + offset, 255, 4);
                *(diff1[y] + offset + 0) = 255;
                *(diff1[y] + offset + 1) = 255;
                *(diff1[y] + offset + 2) = 255;
                *(diff1[y] + offset + 3) = 0;
                // memset(diff2[y] + offset, 255, 4);
                *(diff2[y] + offset + 0) = 255;
                *(diff2[y] + offset + 1) = 255;
                *(diff2[y] + offset + 2) = 255;
                *(diff2[y] + offset + 3) = 0;
            }
            else
            {
                *(diff1[y] + offset + 0) = *(image1[y] + offset + 0); // R
                *(diff1[y] + offset + 1) = *(image1[y] + offset + 1); // G
                *(diff1[y] + offset + 2) = *(image1[y] + offset + 2); // B
                *(diff1[y] + offset + 3) = *(image1[y] + offset + 3); // A
                *(diff2[y] + offset + 0) = *(image2[y] + offset + 0);
                *(diff2[y] + offset + 1) = *(image2[y] + offset + 1);
                *(diff2[y] + offset + 2) = *(image2[y] + offset + 2);
                *(diff2[y] + offset + 3) = *(image2[y] + offset + 3);
            }
        }
    }

    write_png("diff8.png", &ihdr, NULL, NULL, 0, diff1);
    write_png("diff9.png", &ihdr, NULL, NULL, 0, diff2);

    for (y = 0; y < 815; y++)
    {
        free(image1[y]);
        free(image2[y]);
        free(diff1[y]);
        free(diff2[y]);
    }
    free(image1);
    free(image2);
    free(diff1);
    free(diff2);

    return 0;
}
