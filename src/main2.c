
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "countdown2038.h"
#include "pngheaders.h"
#include "pngsample_gennoise.h"
#include "randomsample.h"
#include "roulette.h"
#include "searchAddressFromExistingKeys.h"
#include <CL/opencl.h>
#include <errno.h>
#include <gmp.h>
#include <inttypes.h>
#include <java_random.h>
#include <limits.h>
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
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

#define SECKEY "ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc="

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
 * timer_create+sigeventでタイマーを使ってスレッドを起動する
 * decodable random source?
 */
int hiho(int argc, char **argv, const char **envp)
{
    BIO *mem = BIO_new_mem_buf(SECKEY, strlen(SECKEY));
    BIO *base64 = BIO_new(BIO_f_base64());
    if (base64 == NULL || mem == NULL)
    {
        unsigned long err = ERR_get_error();
        printf("1: %s\n", ERR_error_string(err, NULL));
        return 1;
    }
    // 入力の末尾に改行がない場合はフラグ必須
    BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);
    BIO *io = BIO_push(base64, mem);
    if (io == NULL)
    {
        unsigned long err = ERR_get_error();
        printf("2: %s\n", ERR_error_string(err, NULL));
        return 1;
    }
    unsigned char buf[64] = "";
    int len = 0;
    if ((len = BIO_read(io, buf, 64)) <= 0)
    {
        unsigned long err = ERR_get_error();
        printf("3: %s\n", ERR_error_string(err, NULL));
        BIO_free_all(io);
        return 1;
    }
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\n");
    BIO_free_all(io);

    png_color c1 = { 0 };
    c1.red = 249;
    c1.green = 0;
    c1.blue = 149;
    png_color c2 = { 0 };
    c2.red = 255;
    c2.green = 147;
    c2.blue = 236;
    struct IHDR ihdr = { 0 };
    ihdr.width = 1920;
    ihdr.height = 1080;
    ihdr.bit_depth = 8;
    ihdr.color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    const double h = hypot(1920, 1080);
    png_byte **data = malloc(sizeof(png_byte *) * 1080);
    ssize_t len2 = 0;
    for (size_t y = 0; y < 1080; y++)
    {
        data[y] = malloc(sizeof(png_byte) * 1920 * 4);
        for (size_t x = 0; x < 7680; x += 4)
        {
            data[y][x + 0] = (png_byte)((255. * (1080 - y) + 249 * y) / 1080);
            data[y][x + 1] = (png_byte)((147. * (1080 - y) + 0 * y) / 1080);
            data[y][x + 2] = (png_byte)((236. * (1080 - y) + 149 * y) / 1080);
            data[y][x + 3] = (png_byte)((255. * (h - hypot(1080 - y, x/4))) / h);
        }
    }
    write_png("/mnt/c/Users/terut/OneDrive/Pictures/projects/study/gradation/"
              "gradation12.png",
              &ihdr, NULL, NULL, 0, data);
error:
    for (size_t y = 0; y < 1080; y++)
    {
        free(data[y]);
    }
    free(data);

    return 0;
}
