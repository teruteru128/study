
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "pngheaders.h"
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

#define LIMIT 16

// 2038年問題カウントダウン
static int countdown_2038()
{
    time_t u = 0x80000000L;
    long diff = 0;
    ldiv_t quot = { 0 };
    while (1)
    {
        diff = (long)difftime(u, time(NULL));
        quot = ldiv(diff, 86400L);
        printf("2038年1月19日12時14分8秒まであと%ld日%ld秒\n", quot.quot,
               quot.rem);
        sleep(1);
    }
}

// 乱数的な
static int random_()
{
    uint64_t a = 0;
    ssize_t ret = getrandom(&a, 6, 0);
    if (ret != 6)
    {
        perror("getrandom");
        return 1;
    }
    a = le64toh(a);
    printf("%lf\n", (16 * (a / (double)(1UL << 48))));
    printf("%lf\n", 539. / 540);
    printf("%lf\n", (1 + sqrt(5)) / 3);
    return 0;
}

// 乱数的な
static int random2_()
{
    uint64_t a = 0;
    ssize_t ret = 0;
    uint64_t min = ULONG_MAX;
    do
    {
        ret = getrandom(&a, 6, 0);
        if (ret != 6)
        {
            perror("getrandom");
            return 1;
        }
        if (a == 0)
        {
            min = 0;
            break;
        }
        a = le64toh(a);
        if (a < min)
        {
            min = a;
            printf("%016" PRIx64 "\n", a);
        }
    } while (1);
    return 0;
}

// 乱数的な
static int random3_()
{
    unsigned char a[8];
    ssize_t ret = 0;
    unsigned char reduction = 0;
    size_t i = 0;
    while (1)
    {
        ret = getrandom(&a, 8, 0);
        reduction = a[0] | a[1] | a[2] | a[3] | a[4] | a[5] | a[6] | a[7];
        if ((reduction & 0x0f) == 0x00 || (reduction & 0xf0) == 0x00)
        {
            printf("%02x: ", reduction);
            for (i = 0; i < 8; i++)
            {
                printf("%02x", a[i]);
            }
            printf("\n");
        }
    }
    return 0;
}

static void ma1(uint64_t a)
{
    unsigned long base10;
    unsigned long base16;
    base10 = (unsigned long)log10(a);
    base16 = (unsigned long)(log2(a) / 4);
    printf("%lu - %lu = %lu\n", base10, base16, base10 - base16);
}

static void ma2(uint64_t a)
{
    char buf[22] = "";
    size_t length1 = 0;
    size_t length2 = 0;
    length1 = snprintf(buf, 22, "%lu", a) - 1;
    length2 = snprintf(buf, 22, "%lx", a) - 1;
    printf("%lu - %lu = %lu\n", length1, length2, length1 - length2);
}

/**
 * @brief
 * 10進数表記と16進数表記で2桁違うのはいくらかいくらまでなんやっちゅう話や
 * solve 2 <= log(10, x) - log(16, x) , log(10, x) - log(16, x) < 3
 * 1000000 <= x < 1048576
 * @return int
 */
int fu()
{
    ma1(999999);
    ma1(1000000);
    ma1(0xfffff);
    ma1(0x100000);

    ma1(9999999);
    ma1(10000000);
    ma1(0xffffff);
    ma1(0x1000000);

    ma1(99999999);
    ma1(100000000);
    ma1(0xfffffff);
    ma1(0x10000000);

    ma1(999999999);
    ma1(1000000000);
    ma1(0xffffffffUL);
    ma1(0x100000000UL);

    ma1(9999999999UL);
    ma1(10000000000UL);
    ma1(0xfffffffffUL);
    ma1(0x1000000000UL);

    ma1(99999999999UL);
    ma1(100000000000UL);
    // ここまで16進数の桁数に10進数の桁数に差を詰める
    // log(10, 628288020076) ≒ log(16, 628288020076)
    printf("!!\n");
    // ここは10進数の桁数が差を開く
    ma1(999999999999UL);
    ma1(1000000000000UL);

    // ここから10進数の桁数が16進数の桁数に差を開く
    ma1(0xffffffffffUL);
    ma1(0x10000000000UL);
    ma1(10000000000000UL);
    ma1(10000000000000UL);

    ma1(0xfffffffffffUL);
    ma1(0x100000000000UL);
    ma1(99999999999999UL);
    ma1(100000000000000UL);

    ma1(0xffffffffffffUL);
    ma1(0x1000000000000UL);
    ma1(999999999999999UL);
    ma1(1000000000000000UL);

    ma1(0xfffffffffffffUL);
    ma1(0x10000000000000UL);
    ma1(9999999999999999UL);
    ma1(10000000000000000UL);

    ma2(0xffffffffffffffUL);
    ma2(0x100000000000000UL);
    ma2(99999999999999999UL);
    ma2(100000000000000000UL);

    return 0;
}

int gennoise()
{
    struct IHDR ihdr = { 0 };
    ihdr.width = 1920;
    ihdr.height = 1080;
    ihdr.bit_depth = 8;
    ihdr.color_type = PNG_COLOR_TYPE_RGB;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_byte **data = malloc(sizeof(png_byte *) * 1080);
    ssize_t size = 0;
    size_t x = 0;
    const size_t byteswidth = sizeof(png_byte) * 3 * 1920;
    for (size_t y = 0; y < 1080; y++)
    {
        data[y] = malloc(byteswidth);
        size = getrandom(data[y], byteswidth, 0);
        for (x = 0; x < byteswidth; x++)
        {
            data[y][x] = 255 - (png_byte)(data[y][x] * 0.1875);
        }
    }

    write_png("noise1.png", &ihdr, NULL, NULL, 0,
              data);

    for (size_t y = 0; y < 1080; y++)
    {
        free(data[y]);
    }
    free(data);
    return 0;
}

int roulette(const char **table, const size_t tablesize)
{
    uint64_t a = 0;
    ssize_t s = getrandom(&a, 6, 0);
    if (s != 6)
    {
        perror("getrandom");
        return 1;
    }
    double b = tablesize * (le64toh(a) / (double)(1UL << 48));
    printf("%lf, %s\n", b, table[(size_t)b]);

    return 0;
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
#if 0
    if (argc < 3)
    {
        fprintf(stderr, "%s publickeyfile1 publickeyfile2\n", argv[0]);
    }
    size_t count = 0;
    FILE *pub1 = fopen(argv[1], "rb");
    FILE *pub2 = fopen(argv[2], "rb");
    if (pub1 == NULL || pub2 == NULL)
    {
        perror("fopen");
        if (pub1)
            fclose(pub1);
        if (pub2)
            fclose(pub2);
        return 1;
    }
    struct stat filestat1;
    struct stat filestat2;
    if (fstat(fileno(pub1), &filestat1) != 0
        || fstat(fileno(pub2), &filestat2) != 0)
    {
        perror("fstat");
        fclose(pub1);
        fclose(pub2);
        return 1;
    }
    div_t d1 = div(filestat1.st_size, 65);
    div_t d2 = div(filestat2.st_size, 65);
    if (d1.rem != 0 || d2.rem != 0)
    {
        fprintf(
            stderr,
            "%sもしくは%"
            "sのどちらかのファイルサイズが公開鍵サイズの倍数ではありません\n",
            argv[1], argv[2]);
        fclose(pub1);
        fclose(pub2);
        return 1;
    }
    // 16件ずつローカル変数にコピーするのって面倒じゃない……？
    unsigned char a[1040];
    unsigned char b[1040];
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    EVP_MD_CTX *ctx0 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx2 = EVP_MD_CTX_new();
    size_t x;
    size_t y;
    size_t i;
    size_t j;
    EVP_DigestInit_ex2(ctx1, EVP_sha512(), NULL);
    unsigned char hash[EVP_MAX_MD_SIZE];
    EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
    if (ripemd160 == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "ripemd160 : %s\n", ERR_error_string(err, NULL));
        return 1;
    }
    for (x = 0; x < d2.quot; x += 16)
    {
        fread(a, 65, 16, pub1);
        for (y = 0; y < d1.quot; y += 16)
        {
            fread(b, 65, 16, pub2);
            for (i = 0; i < 1040; i += 65)
            {
                EVP_DigestInit_ex2(ctx0, sha512, NULL);
                EVP_DigestUpdate(ctx0, a + i, 65);
                for (j = 0; j < 1040; j += 65)
                {
                    EVP_MD_CTX_copy(ctx1, ctx0);
                    EVP_DigestUpdate(ctx1, b + j, 65);
                    EVP_DigestFinal_ex(ctx1, hash, NULL);
                    EVP_DigestInit_ex2(ctx2, ripemd160, NULL);
                    EVP_DigestUpdate(ctx2, hash, 64);
                    EVP_DigestFinal_ex(ctx2, hash, NULL);
                    // hash[0] == 0
                    if (!hash[0])
                    {
                        printf("%zu, %zu\n", x + (i / 65), y + (j / 65));
                        count++;
                        if (count >= LIMIT)
                        {
                            goto finish;
                        }
                    }
                }
            }
        }
    }
finish:
    EVP_MD_free(sha512);
    EVP_MD_free(ripemd160);
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);

    EVP_MD_CTX_free(ctx0);
    EVP_MD_CTX_free(ctx1);
    fclose(pub1);
    fclose(pub2);
#endif
    // countdown_2038();
    // fu();
    // random3_();
    const char *r[] = { "がび君", "左近君", "無人島君" };
    roulette(r, 3);
    printf("%p\n", r);
    printf("%p, %p, %p\n", &r[0], &r[1], &r[2]);
    printf("%p, %p, %p\n", &r[0][0], &r[1][0], &r[2][0]);
    char *f = malloc(16);
    printf("%p\n", f);
    free(f);
    f = alloca(16);
    printf("%p\n", f);
    return 0;
}
