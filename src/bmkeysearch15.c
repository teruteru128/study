
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "timeutil.h"
#include <bm.h>
#include <fcntl.h>
#include <omp.h>
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

#define CTX_CACHE_SIZE 16
#define ENC_CACHE_SIZE 16
#define ENC_NUM 67108864UL

#define LOCAL_CACHE_NUM 16

static int loadKey1(unsigned char *publicKey, const char *path, size_t size,
                    size_t num)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    FILE *publicKeyFile = fopen(path, "rb");
    if (publicKeyFile == NULL)
    {
        if (publicKeyFile != NULL)
        {
            fclose(publicKeyFile);
        }
        return 1;
    }
    size_t pubnum = fread(publicKey, size, num, publicKeyFile);
    fclose(publicKeyFile);
    if (pubnum != num)
    {
        perror("fread");
        free(publicKey);
        return 1;
    }
    return 0;
}

static int loadPrivateKey1(unsigned char *publicKey, const char *path)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    return loadKey1(publicKey, path, 32, 16777216);
}

static int loadPublicKey1(unsigned char *publicKey, const char *path)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    return loadKey1(publicKey, path, 65, 16777216);
}

static int dappunda(const EVP_MD *sha512, const EVP_MD *ripemd160)
{
    unsigned char *publicKeyGlobal = malloc(16777216UL * 65 * 4);
    if (loadPublicKey1(
            publicKeyGlobal,
            "/home/teruteru128/git/study/keys/public/publicKeys0.bin")
        || loadPublicKey1(
            publicKeyGlobal + 16777216UL * 65,
            "/home/teruteru128/git/study/keys/public/publicKeys1.bin")
        || loadPublicKey1(
            publicKeyGlobal + 16777216UL * 65 * 2,
            "/home/teruteru128/git/study/keys/public/publicKeys2.bin")
        || loadPublicKey1(
            publicKeyGlobal + 16777216UL * 65 * 3,
            "/home/teruteru128/git/study/keys/public/publicKeys3.bin"))
    {
        perror("publickey");
        return 1;
    }
    unsigned char *privateKeyGlobal = malloc(16777216UL * 32 * 4);
    if (loadPrivateKey1(
            privateKeyGlobal,
            "/home/teruteru128/git/study/keys/private/privateKeys0.bin")
        || loadPrivateKey1(
            privateKeyGlobal + 16777216UL * 32,
            "/home/teruteru128/git/study/keys/private/privateKeys1.bin")
        || loadPrivateKey1(
            privateKeyGlobal + 16777216UL * 32 * 2,
            "/home/teruteru128/git/study/keys/private/privateKeys2.bin")
        || loadPrivateKey1(
            privateKeyGlobal + 16777216UL * 32 * 3,
            "/home/teruteru128/git/study/keys/private/privateKeys3.bin"))
    {
        perror("privatekey");
        return 1;
    }
    // sign側のMD_CTXを複数にしてみる
#pragma omp parallel default(none)                                            \
    shared(publicKeyGlobal, privateKeyGlobal, sha512, ripemd160, stderr)
    {
        unsigned char hash[EVP_MAX_MD_SIZE];
        char *address = NULL;
        char *sigwif = NULL;
        char *encwif = NULL;
        EVP_MD_CTX *shactx1[CTX_CACHE_SIZE] = { NULL };
        for (size_t i = 0; i < CTX_CACHE_SIZE; i++)
        {
            shactx1[i] = EVP_MD_CTX_new();
        }
        EVP_MD_CTX *shactx2 = EVP_MD_CTX_new();
        EVP_MD_CTX *ripectx = EVP_MD_CTX_new();
        EVP_DigestInit_ex2(shactx2, sha512, NULL);
        EVP_DigestInit_ex2(ripectx, ripemd160, NULL);
        unsigned char encbuf[ENC_CACHE_SIZE * PUBLIC_KEY_LENGTH];
        size_t sigindex = 0;
        size_t encindex = 0;
        size_t encglobalindex = 0;
        size_t encoffset = 0;
        size_t sigglobalindex = 0;
        struct timespec startspec;
        struct timespec finishspec;
        struct timespec diffspec;
        if (getrandom(&sigglobalindex, 3, 0) != 3)
        {
            goto fail;
        }
        sigglobalindex = (le64toh(sigglobalindex) >> 2) << 4;
        // 128 を 8スレ-> 10分
        // 1536-256=1280 を 8スレ-> 100分
        /*
         * 128 8スレ 10分
         * 16 1スレ 10分
         * 60 1スレ 60分
         * 960 16スレ 60分
         * 12480 16スレ 13時間
         * 19968 16スレ 20時間48分
         * --
         * 1280 8スレ 100分
         * 768 8スレ 60分
         * 1536 16スレ 60分
         * --
         * 16スレッドで回すより8スレで回したほうが1スレッドあたりの速度が早いのね……
         */
        for (; sigglobalindex < ENC_NUM; sigglobalindex += CTX_CACHE_SIZE)
        {
            clock_gettime(CLOCK_MONOTONIC, &startspec);
            for (sigindex = 0; sigindex < CTX_CACHE_SIZE; sigindex++)
            {
                EVP_DigestInit_ex2(shactx1[sigindex], sha512, NULL);
                EVP_DigestUpdate(shactx1[sigindex],
                                 (PublicKey *)publicKeyGlobal + sigglobalindex
                                     + sigindex,
                                 PUBLIC_KEY_LENGTH);
            }
            for (encglobalindex = 0; encglobalindex < ENC_NUM;
                 encglobalindex += ENC_CACHE_SIZE)
            {
                memcpy(encbuf, (PublicKey *)publicKeyGlobal + encglobalindex,
                       ENC_CACHE_SIZE * PUBLIC_KEY_LENGTH);
                for (encindex = 0, encoffset = 0; encindex < ENC_CACHE_SIZE;
                     encindex++, encoffset += PUBLIC_KEY_LENGTH)
                {
                    for (sigindex = 0; sigindex < CTX_CACHE_SIZE; sigindex++)
                    {
                        EVP_MD_CTX_copy_ex(shactx2, shactx1[sigindex]);
                        EVP_DigestUpdate(shactx2, encbuf + encoffset,
                                         PUBLIC_KEY_LENGTH);
                        EVP_DigestFinal_ex(shactx2, hash, NULL);
                        // init_ex2とcopy_exってどっちが早いんやろうな？
                        // 全部コピーするからcopy_exのほうが遅いのかもしれん
                        EVP_DigestInit_ex2(ripectx, ripemd160, NULL);
                        EVP_DigestUpdate(ripectx, hash, 64);
                        EVP_DigestFinal_ex(ripectx, hash, NULL);
                        // GPUで計算するときはハッシュだけGPUで計算して
                        // チェックとフォーマットはCPUでやったほうがいいのかなあ？
                        // htobe64(*(unsigned long *)hash) ==
                        // 0xffffffffffff0000UL
                        if ((*(unsigned long *)hash) & 0x0000ffffffffffffUL)
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
            clock_gettime(CLOCK_MONOTONIC, &finishspec);
            difftimespec(&diffspec, &finishspec, &startspec);
#pragma omp critical
            fprintf(stderr, "%lf addresses/seconds\n",
                    ((double)CTX_CACHE_SIZE * ENC_NUM * 1000000000UL)
                        / (diffspec.tv_sec * 1000000000UL + diffspec.tv_nsec));
#pragma omp barrier
        }
    fail:
        for (size_t i = 0; i < CTX_CACHE_SIZE; i++)
        {
            EVP_MD_CTX_free(shactx1[i]);
        }
        EVP_MD_CTX_free(shactx2);
        EVP_MD_CTX_free(ripectx);
    }
    return 0;
}

int searchAddressFromExistingKeys3()
{
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
    dappunda(sha512, ripemd160);
    EVP_MD_free(sha512);
    EVP_MD_free(ripemd160);
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
    return 0;
}

int main(int argc, char const *argv[])
{
    return searchAddressFromExistingKeys3();
}
