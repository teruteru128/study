
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
#include <bm.h>
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

#define LOCAL_CACHE_NUM 16

/**
 * @brief
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
 *
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int hiho(int argc, char **argv, const char **envp)
{
    FILE *publicKeyFile = fopen(
        "/home/teruteru128/git/study/keys/public/publicKeys0.bin", "rb");
    FILE *privateKeyFile = fopen(
        "/home/teruteru128/git/study/keys/private/privateKeys0.bin", "rb");
    if (publicKeyFile == NULL || privateKeyFile == NULL)
    {
        if (publicKeyFile != NULL)
        {
            fclose(publicKeyFile);
        }
        if (privateKeyFile != NULL)
        {
            fclose(privateKeyFile);
        }
        return 1;
    }
    unsigned char *publicKeyGlobal = malloc(1090519040L);
    unsigned char *privateKeyGlobal = malloc(536870912L);
    size_t pubnum = fread(publicKeyGlobal, 65, 16777216, publicKeyFile);
    size_t prinum = fread(privateKeyGlobal, 32, 16777216, privateKeyFile);
    fclose(publicKeyFile);
    fclose(privateKeyFile);
    if (pubnum != 16777216 || prinum != 16777216)
    {
        perror("fread");
        return 1;
    }
    size_t sigglobalindex = 0;
    size_t sigglobaloffset = 0;
    size_t sigindex = 0;
    size_t sigindexmax = 0;
    size_t sigoffset = 0;
    size_t encglobalindex = 0;
    size_t encglobaloffset = 0;
    size_t encindex = 0;
    size_t encindexmax = 0;
    size_t encoffset = 0;
    EVP_MD_CTX *shactx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *shactx2 = EVP_MD_CTX_new();
    EVP_MD_CTX *ripectx = EVP_MD_CTX_new();
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
    unsigned char sigbuf[LOCAL_CACHE_NUM * 65];
    unsigned char encbuf[LOCAL_CACHE_NUM * 65];
    unsigned char hash[EVP_MAX_MD_SIZE];
    size_t count = 0;
    char *address = NULL;
    char *sigwif = NULL;
    char *encwif = NULL;
    EVP_DigestInit_ex2(shactx2, sha512, NULL);
    if (ripemd160 == NULL)
    {
        fprintf(stderr, "ripemd160 is not found\n");
        return 1;
    }
    for (sigglobalindex = 0, sigglobaloffset = 0; sigglobalindex < 16;
         sigglobalindex += LOCAL_CACHE_NUM, sigglobaloffset += LOCAL_CACHE_NUM * 65)
    {
        memcpy(sigbuf, publicKeyGlobal + sigglobaloffset, LOCAL_CACHE_NUM * 65);
        for (encglobalindex = 0, encglobaloffset = 0;
             encglobalindex < 1024;
             encglobalindex += LOCAL_CACHE_NUM, encglobaloffset += LOCAL_CACHE_NUM * 65)
        {
            memcpy(encbuf, publicKeyGlobal + encglobaloffset, LOCAL_CACHE_NUM * 65);
            for (sigindex = 0, sigoffset = 0; sigindex < LOCAL_CACHE_NUM;
                 sigindex++, sigoffset += 65)
            {
                EVP_DigestInit_ex2(shactx1, sha512, NULL);
                EVP_DigestUpdate(shactx1, sigbuf + sigoffset, 65);
                for (encindex = 0, encoffset = 0; encindex < LOCAL_CACHE_NUM;
                     encindex++, encoffset += 65)
                {
                    EVP_MD_CTX_copy_ex(shactx2, shactx1);
                    EVP_DigestUpdate(shactx2, encbuf + encoffset, 65);
                    EVP_DigestFinal(shactx2, hash, NULL);
                    EVP_DigestInit_ex2(ripectx, ripemd160, NULL);
                    EVP_DigestUpdate(ripectx, hash, 64);
                    EVP_DigestFinal(ripectx, hash, NULL);
                    if (hash[0] == 0)
                    {
                        address = encodeV4Address(hash, 20);
                        sigwif = encodeWIF((
                            PrivateKey *)(privateKeyGlobal
                                          + (sigglobalindex + sigindex) * 32));
                        encwif = encodeWIF((
                            PrivateKey *)(privateKeyGlobal
                                          + (encglobalindex + encindex) * 32));
                        printf("%s,%s,%s\n", address, sigwif, encwif);
                        free(address);
                        free(sigwif);
                        free(encwif);
                        count++;
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

    free(publicKeyGlobal);
    free(privateKeyGlobal);
    return 0;
}
