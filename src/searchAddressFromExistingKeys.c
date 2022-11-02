
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <bm.h>
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

#define LIMIT 16

/**
 * @brief bitmessage鍵セット探索
 * 既存鍵を組み合わせるのってメチャクチャ面倒くさくない？
 *
 * @param argc
 * @param argv
 * @return int
 */
int searchAddressFromExistingKeys(int argc, char **argv)
{
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
    size_t c;
    for (x = 0; x < d2.quot; x += 16)
    {
        c = fread(a, 65, 16, pub1);
        if (c != 16)
        {
            goto finish;
        }
        for (y = 0; y < d1.quot; y += 16)
        {
            c = fread(b, 65, 16, pub2);
            if (c != 16)
            {
                goto finish;
            }
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
    return 0;
}

#define LOCAL_CACHE_NUM 16

static int deepdarkfantasy()
{
    unsigned char *publicKeyGlobal = malloc(1090519040UL);
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
            // 24bitから下7bit切り捨て->24bit乱数から7bit切って7bit底上げ
            // 24bitから下8bit切り捨て->16bit乱数から8bit底上げ
            // 24bitから下10bit切り捨て->16bit乱数から2bit切って10bit底上げ
            // [0, 65536) -> [0, 16384) -> [0, 16777216)(unit 256)
            sigglobalindex = le64toh(sigglobalindex) << 8;
            sigglobalindexmax = sigglobalindex + 256;
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
            fprintf(stderr, "%zu->%zu done (%ld)\n", sigglobalindexmax - 256,
                    sigglobalindexmax, time(NULL));
#pragma omp barrier
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

int searchAddressFromExistingKeys2()
{
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    deepdarkfantasy();
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
    return 0;
}

static int dappunda() {
    // sign側のMD_CTXを複数にしてみる
}

int searchAddressFromExistingKeys3()
{
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    dappunda();
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
    return 0;
}
