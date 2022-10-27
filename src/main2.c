
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
#include <fcntl.h>
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
#include <sys/mman.h>
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

int deepdarkfantasy()
{
    unsigned char *publicKeyGlobal = malloc(1090519040L);
    unsigned char *privateKeyGlobal = NULL;
    {
        // public keyは頻繁に使うのでメモリに読み込んでおく
        FILE *publicKeyFile = fopen(
            "/home/teruteru128/git/study/keys/public/publicKeys0.bin", "rb");
        if (publicKeyFile == NULL)
        {
            if (publicKeyFile != NULL)
            {
                fclose(publicKeyFile);
            }
            return 1;
        }
        size_t pubnum = fread(publicKeyGlobal, 65, 16777216, publicKeyFile);
        fclose(publicKeyFile);
        if (pubnum != 16777216)
        {
            perror("fread");
            free(publicKeyGlobal);
            return 1;
        }
    }
    {
        size_t page_size = sysconf(_SC_PAGESIZE);
        // マッピングサイズはページングサイズ単位で切り上げ
        size_t map_size
            = ((536870912UL + page_size - 1) / page_size) * page_size;
        int privateKeyFD
            = open("/home/teruteru128/git/study/keys/private/privateKeys0.bin",
                   O_RDONLY);
        if (privateKeyFD < 0)
        {
            perror("open");
            return 1;
        }
        // private keyはめったに使わないのでmmapで済ます
        privateKeyGlobal
            = mmap(NULL, map_size, PROT_READ, MAP_SHARED, privateKeyFD, 0);
        if (privateKeyGlobal == MAP_FAILED)
        {
            perror("mmap");
            close(privateKeyFD);
            free(publicKeyGlobal);
            return 1;
        }
        // ファイルディスクリプターは使わないので閉じておく
        close(privateKeyFD);
    }

    size_t sigglobalindex = 0;
    size_t sigglobalindexmax = 0;
    size_t sigindex = 0;
    size_t sigoffset = 0;
    size_t encglobalindex = 0;
    size_t encglobalindexmax = 0;
    size_t encindex = 0;
    size_t encoffset = 0;
    EVP_MD_CTX *shactx1 = NULL;
    EVP_MD_CTX *shactx2 = NULL;
    EVP_MD_CTX *ripectx = NULL;
    EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
    unsigned char sigbuf[LOCAL_CACHE_NUM * 65];
    unsigned char encbuf[LOCAL_CACHE_NUM * 65];
    unsigned char hash[EVP_MAX_MD_SIZE];
    char *address = NULL;
    char *sigwif = NULL;
    char *encwif = NULL;
    if (ripemd160 == NULL)
    {
        fprintf(stderr, "ripemd160 is not found\n");
        return 1;
    }
    size_t count = 0;
#pragma omp parallel private(sigglobalindex, sigglobalindexmax, sigindex,     \
                             sigoffset, encglobalindex, encglobalindexmax,    \
                             encindex, encoffset, shactx1, shactx2, ripectx,  \
                             sigbuf, encbuf, hash, address, sigwif, encwif)
    {
        shactx1 = EVP_MD_CTX_new();
        shactx2 = EVP_MD_CTX_new();
        ripectx = EVP_MD_CTX_new();
        EVP_DigestInit_ex2(shactx2, sha512, NULL);
        while (1)
        {
            // 0で埋めないと高位bitにデータが残ったままになる
            sigglobalindex = 0;
            if (getrandom(&sigglobalindex, 2, 0) != 2)
            {
                break;
            }
            // [0, 65536) -> [0, 16384) -> [0, 16777216)(unit 1024)
            sigglobalindex = (le64toh(sigglobalindex) >> 2) << 10;
            sigglobalindexmax = sigglobalindex + 1024;
#pragma omp critical
            fprintf(stderr, "%zu->%zu (%ld)\n", sigglobalindex,
                    sigglobalindexmax, time(NULL));
            for (; sigglobalindex < sigglobalindexmax;
                 sigglobalindex += LOCAL_CACHE_NUM)
            {
                memcpy(sigbuf,
                       publicKeyGlobal + (sigglobalindex << 6)
                           + sigglobalindex,
                       LOCAL_CACHE_NUM * 65);
                for (encglobalindex = 0; encglobalindex < 16777216;
                     encglobalindex += LOCAL_CACHE_NUM)
                {
                    memcpy(encbuf,
                           publicKeyGlobal + (encglobalindex << 6)
                               + encglobalindex,
                           LOCAL_CACHE_NUM * 65);
                    for (sigindex = 0, sigoffset = 0;
                         sigindex < LOCAL_CACHE_NUM;
                         sigindex++, sigoffset += 65)
                    {
                        EVP_DigestInit_ex2(shactx1, sha512, NULL);
                        EVP_DigestUpdate(shactx1, sigbuf + sigoffset, 65);
                        for (encindex = 0, encoffset = 0;
                             encindex < LOCAL_CACHE_NUM;
                             encindex++, encoffset += 65)
                        {
                            EVP_MD_CTX_copy_ex(shactx2, shactx1);
                            EVP_DigestUpdate(shactx2, encbuf + encoffset, 65);
                            EVP_DigestFinal_ex(shactx2, hash, NULL);
                            EVP_DigestInit_ex2(ripectx, ripemd160, NULL);
                            EVP_DigestUpdate(ripectx, hash, 64);
                            EVP_DigestFinal_ex(ripectx, hash, NULL);
                            // GPUで計算するときはハッシュだけGPUで計算して
                            // チェックとフォーマットはCPUでやったほうがいいのかなあ？
                            // htobe64(*(unsigned long *)hash) ==
                            // 0xffffffffffff0000UL
                            if ((*(unsigned long *)hash)
                                & 0x0000ffffffffffffUL)
                            {
                                continue;
                            }
                            address = encodeV4Address(hash, 20);
                            sigwif = encodeWIF((PrivateKey *)privateKeyGlobal
                                               + sigglobalindex + sigindex);
                            encwif = encodeWIF((PrivateKey *)privateKeyGlobal
                                               + encglobalindex + encindex);
#pragma omp critical
                            printf("%s,%s,%s\n", address, sigwif, encwif);
                            free(address);
                            free(sigwif);
                            free(encwif);
                        }
                    }
                }
            }
#pragma omp critical
            fprintf(stderr, "%zu->%zu done (%ld)\n", sigglobalindexmax - 1024,
                    sigglobalindexmax, time(NULL));
        }
        EVP_MD_CTX_free(shactx1);
        EVP_MD_CTX_free(shactx2);
        EVP_MD_CTX_free(ripectx);
    }
finish:
    EVP_MD_free(sha512);
    EVP_MD_free(ripemd160);

    free(publicKeyGlobal);
    munmap(privateKeyGlobal, 536870912UL);
    return 0;
}

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
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    deepdarkfantasy();
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
    return 0;
}
