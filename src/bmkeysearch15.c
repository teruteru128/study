
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
#include <regex.h>
#include <signal.h>
#include <stdatomic.h>
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
    return loadKey1(publicKey, path, 32, 16777216);
}

static int loadPublicKey1(unsigned char *publicKey, const char *path)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    return loadKey1(publicKey, path, 64, 16777216);
}

static volatile sig_atomic_t running = 1;

static void handler(int sig, siginfo_t *info, void *ctx)
{
    running = 0;
    (void)sig;
    (void)info;
    (void)ctx;
}

static int dappunda(const EVP_MD *sha512, const EVP_MD *ripemd160)
{
    unsigned char *publicKeyGlobal = NULL;
    int en = 0;
    if ((en = posix_memalign((void **)&publicKeyGlobal, sysconf(_SC_PAGESIZE),
                             16777216UL * 64 * 4))
        != 0)
    {
        fprintf(stderr, "posix_memalign: %s\n", strerror(en));
        return 1;
    }
    if (loadPublicKey1(publicKeyGlobal,
                       "/mnt/d/keys/public/trimmed/publicKeys0.bin")
        || loadPublicKey1(publicKeyGlobal + 16777216UL * 64,
                          "/mnt/d/keys/public/trimmed/publicKeys1.bin")
        || loadPublicKey1(publicKeyGlobal + 16777216UL * 64 * 2,
                          "/mnt/d/keys/public/trimmed/publicKeys2.bin")
        || loadPublicKey1(publicKeyGlobal + 16777216UL * 64 * 3,
                          "/mnt/d/keys/public/trimmed/publicKeys3.bin"))
    {
        perror("publickey");
        return 1;
    }
    unsigned char *privateKeyGlobal = malloc(16777216UL * 32 * 4);
    if (loadPrivateKey1(privateKeyGlobal,
                        "/mnt/d/keys/private/privateKeys0.bin")
        || loadPrivateKey1(privateKeyGlobal + 16777216UL * 32,
                           "/mnt/d/keys/private/privateKeys1.bin")
        || loadPrivateKey1(privateKeyGlobal + 16777216UL * 32 * 2,
                           "/mnt/d/keys/private/privateKeys2.bin")
        || loadPrivateKey1(privateKeyGlobal + 16777216UL * 32 * 3,
                           "/mnt/d/keys/private/privateKeys3.bin"))
    {
        perror("privatekey");
        return 1;
    }
    // sign側のMD_CTXを複数にしてみる
#pragma omp parallel default(none)                                            \
    shared(publicKeyGlobal, privateKeyGlobal, sha512, ripemd160, stderr,      \
           stdout, running)
    {
        unsigned char hash[EVP_MAX_MD_SIZE];
        char *address = NULL;
        char *sigwif = NULL;
        char *encwif = NULL;
        const unsigned char prefix = 4;
        EVP_MD_CTX *shactx1[CTX_CACHE_SIZE] = { NULL };
        for (size_t i = 0; i < CTX_CACHE_SIZE; i++)
        {
            shactx1[i] = EVP_MD_CTX_new();
        }
        EVP_MD_CTX *shactx2 = EVP_MD_CTX_new();
        EVP_MD_CTX *ripectx = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(shactx2, sha512, NULL);
        EVP_DigestInit_ex2(ripectx, ripemd160, NULL);
#else
        EVP_DigestInit_ex(shactx2, sha512, NULL);
        EVP_DigestInit_ex(ripectx, ripemd160, NULL);
#endif
        unsigned char encbuf[ENC_CACHE_SIZE * 64];
        size_t sigindex = 0;
        size_t encindex = 0;
        size_t encglobalindex = 0;
        size_t encoffset = 0;
        size_t sigglobalindex = 0;
        double startwtime;
        double finishwtime;
        double diffwtime;
        if (getrandom(&sigglobalindex, 3, 0) != 3)
        {
            goto fail;
        }
        sigglobalindex = (le64toh(sigglobalindex) >> 2) << 4;
        // 128 を 8スレ-> 10分
        // 1536-256=1280 を 8スレ-> 100分
        /*
         *  8スレッド: 864901.441618 addresses/seconds
         * 16スレッド: 701640.168877 addresses/seconds
         * 16スレッドで回すより8スレで回したほうが1スレッドあたりの速度が早いのね……
         */
        for (; running && sigglobalindex < ENC_NUM;
             sigglobalindex += CTX_CACHE_SIZE)
        {
            startwtime = omp_get_wtime();
            for (sigindex = 0; sigindex < CTX_CACHE_SIZE; sigindex++)
            {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
                EVP_DigestInit_ex2(shactx1[sigindex], sha512, NULL);
#else
                EVP_DigestInit_ex(shactx1[sigindex], sha512, NULL);
#endif
                EVP_DigestUpdate(shactx1[sigindex], &prefix, 1);
                EVP_DigestUpdate(
                    shactx1[sigindex],
                    publicKeyGlobal + ((sigglobalindex + sigindex) << 6), 64);
                EVP_DigestUpdate(shactx1[sigindex], &prefix, 1);
            }
            // ここでparallel forを切ってもいいのかもしれない
            for (encglobalindex = 0; encglobalindex < ENC_NUM;
                 encglobalindex += ENC_CACHE_SIZE)
            {
                memcpy(encbuf, publicKeyGlobal + (encglobalindex << 6),
                       ENC_CACHE_SIZE * 64);
                for (encindex = 0, encoffset = 0; encindex < ENC_CACHE_SIZE;
                     encindex++, encoffset += 64)
                {
                    for (sigindex = 0; sigindex < CTX_CACHE_SIZE; sigindex++)
                    {
                        EVP_MD_CTX_copy_ex(shactx2, shactx1[sigindex]);
                        EVP_DigestUpdate(shactx2, encbuf + encoffset, 64);
                        EVP_DigestFinal_ex(shactx2, hash, NULL);
                        // init_ex2とcopy_exってどっちが早いんやろうな？
                        // 全部コピーするからcopy_exのほうが遅いのかもしれん
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
                        EVP_DigestInit_ex2(ripectx, ripemd160, NULL);
#else
                        EVP_DigestInit_ex(ripectx, ripemd160, NULL);
#endif
                        EVP_DigestUpdate(ripectx, hash, 64);
                        EVP_DigestFinal_ex(ripectx, hash, NULL);
                        // GPUで計算するときはハッシュだけGPUで計算して
                        // チェックとフォーマットはCPUでやったほうがいいのかなあ？
                        // be64toh(*(unsigned long *)hash) &
                        // 0xffffffffffff0000UL
                        if ((*(unsigned long *)hash) & 0x000000ffffffffffUL)
                        {
                            continue;
                        }
                        address = encodeV4Address(hash, 20);
                        sigwif = encodeWIF((PrivateKey *)privateKeyGlobal
                                           + sigglobalindex + sigindex);
                        encwif = encodeWIF((PrivateKey *)privateKeyGlobal
                                           + encglobalindex + encindex);
#pragma omp critical
                        {
                            fprintf(stdout, "%s,%s,%s,%d,%ld\n", address,
                                    sigwif, encwif,
                                    __builtin_clzl(
                                        be64toh(*(unsigned long *)hash)),
                                    time(NULL));
                            fflush(stdout);
                        }
                        free(address);
                        free(sigwif);
                        free(encwif);
                    }
                }
            }
            finishwtime = omp_get_wtime();
            diffwtime = finishwtime - startwtime;
#pragma omp critical
            {
                fprintf(stderr, "%lf seconds, %lf addresses/seconds\n",
                        diffwtime,
                        ((double)CTX_CACHE_SIZE * ENC_NUM) / diffwtime);
            }
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

#if 0
// アドレスripe比較用(qsort_r 向け)コンパレータ
int compar(const void *a, const void *b, void *arg)
{
    unsigned long d = *(unsigned long *)a - *(unsigned long *)b;
    if (d < 0)
    {
        return 1;
    }
    else if (d == 0)
    {
        return memcmp(a, b, 20);
    }
    else
    {
        return -1;
    }
}
#endif

int searchAddressFromExistingKeys3()
{
    fprintf(stderr, "my pid is %d\n", getpid());
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
#else
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
#endif
    struct sigaction action = { 0 };
    struct sigaction oact = { 0 };
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;
    int ret = EXIT_SUCCESS;
    if (sigaction(SIGINT, &action, &oact) != 0)
    {
        perror("sigaction(SIGINT)");
        ret = EXIT_FAILURE;
    }
    else
    {
        dappunda(sha512, ripemd160);
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD_free(sha512);
    EVP_MD_free(ripemd160);
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    return ret;
}

int main(int argc, char const *argv[])
{
    return searchAddressFromExistingKeys3();
}
