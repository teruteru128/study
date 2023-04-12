
#define _GNU_SOURCE 1
#define _DEFAULT_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "biom.h"
#include "countdown.h"
#include "countdown2038.h"
#include "pngheaders.h"
#include "pngsample_gennoise.h"
#include "queue.h"
#include "randomsample.h"
#include "roulette.h"
#include "searchAddressFromExistingKeys.h"
#include "timeutil.h"
#include <CL/opencl.h>
#include <bm.h>
#include <complex.h>
#include <ctype.h>
#include <curl/curl.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <gmp.h>
#include <iconv.h>
#include <inttypes.h>
#include <java_random.h>
#include <jsonrpc-glib.h>
#include <limits.h>
#include <locale.h>
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
#include <openssl/ssl.h>
#include <png.h>
#include <printaddrinfo.h>
#include <regex.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include <jpeglib.h> // jpeglibはstdioより下(FILEが依存しているため)

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/provider.h>
#include <openssl/types.h>
#endif

void *task(void *)
{
    uuid_t uuid;
    char passphrase[UUID_STR_LEN] = "";
    long signingKeyNonce = 0;
    long encryptionKeyNonce = 1;
    size_t counter = 0;
    unsigned char nonce[9] = { 0x00 };
    size_t nonce_length = 0;
    unsigned char *nonce1 = nonce + 1;
    unsigned long tmp = 0;
    int nice = 20;
    unsigned char potentialPrivSigningKey[64];
    unsigned char potentialPrivEncryptionKey[64];
    unsigned char potentialPubSigningKey[65];
    unsigned char potentialPubEncryptionKey[65];
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    BN_CTX *bnctx = BN_CTX_new();
    BN_CTX_start(bnctx);
    BIGNUM *prikey = BN_CTX_get(bnctx);
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *sha512ctx0 = EVP_MD_CTX_new();
    EVP_MD_CTX *sha512ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *sha512ctx2 = EVP_MD_CTX_new();
    EVP_MD_CTX *ripemd160ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_DigestInit_ex2(sha512ctx1, sha512, NULL);
    EVP_DigestInit_ex2(sha512ctx2, sha512, NULL);
#else
    EVP_DigestInit_ex(sha512ctx1, sha512, NULL);
    EVP_DigestInit_ex(sha512ctx2, sha512, NULL);
#endif
    do
    {
        // passphrase generate
        uuid_generate_random(uuid);
        uuid_unparse_lower(uuid, passphrase);
        nice = 20;
        counter = 0;
        // private keys generate
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(sha512ctx0, sha512, NULL);
#else
        EVP_DigestInit_ex(sha512ctx0, sha512, NULL);
#endif
        EVP_DigestUpdate(sha512ctx0, passphrase, 36);
        do
        {
            // TODO 公開鍵導出を並列化(parallel+sections)

            if (counter < 253)
            {
                nonce[0] = (unsigned char)counter;
                nonce_length = 1;
            }
            else if (counter < 65536)
            {
                nonce[0] = 0xfd;
                *((uint16_t *)nonce1) = htobe16((uint16_t)counter);
                nonce_length = 3;
            }
            else if (counter < 4294967296L)
            {
                nonce[0] = 0xfe;
                *((uint32_t *)nonce1) = htobe16((uint32_t)counter);
                nonce_length = 5;
            }
            else if (counter <= 18446744073709551615UL)
            {
                nonce[0] = 0xff;
                *((uint64_t *)nonce1) = htobe16((uint64_t)counter);
                nonce_length = 9;
            }

            EVP_MD_CTX_copy_ex(sha512ctx1, sha512ctx0);
            EVP_DigestUpdate(sha512ctx1, nonce, nonce_length);
            EVP_DigestFinal_ex(sha512ctx1, potentialPrivSigningKey, NULL);

            counter++;
            if (counter < 253)
            {
                nonce[0] = (unsigned char)counter;
                nonce_length = 1;
            }
            else if (counter < 65536)
            {
                nonce[0] = 0xfd;
                *((uint16_t *)nonce1) = htobe16((uint16_t)counter);
                nonce_length = 3;
            }
            else if (counter < 4294967296L)
            {
                nonce[0] = 0xfe;
                *((uint32_t *)nonce1) = htobe16((uint32_t)counter);
                nonce_length = 5;
            }
            else if (counter <= 18446744073709551615UL)
            {
                nonce[0] = 0xff;
                *((uint64_t *)nonce1) = htobe16((uint64_t)counter);
                nonce_length = 9;
            }

            EVP_MD_CTX_copy_ex(sha512ctx2, sha512ctx0);
            EVP_DigestUpdate(sha512ctx2, nonce, nonce_length);
            EVP_DigestFinal_ex(sha512ctx2, potentialPrivEncryptionKey, NULL);

            // public keys generate
            BN_bin2bn(potentialPrivSigningKey, 32, prikey);
            EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, bnctx);
            EC_POINT_point2oct(secp256k1, pubkey,
                               POINT_CONVERSION_UNCOMPRESSED,
                               potentialPubSigningKey, 65, bnctx);

            BN_bin2bn(potentialPrivEncryptionKey, 32, prikey);
            EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, bnctx);
            EC_POINT_point2oct(secp256k1, pubkey,
                               POINT_CONVERSION_UNCOMPRESSED,
                               potentialPubEncryptionKey, 65, bnctx);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
            EVP_DigestInit_ex2(sha512ctx1, sha512, NULL);
#else
            EVP_DigestInit_ex(sha512startctx1, sha512, NULL);
#endif
            // ripe generate
            EVP_DigestUpdate(sha512ctx1, potentialPubSigningKey, 65);
            EVP_DigestUpdate(sha512ctx1, potentialPubEncryptionKey, 65);
            EVP_DigestFinal(sha512ctx1, hash, NULL);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
            EVP_DigestInit_ex2(ripemd160ctx, ripemd160, NULL);
#else
            EVP_DigestInit_ex(ripemd160ctx, ripemd160, NULL);
#endif
            EVP_DigestUpdate(ripemd160ctx, hash, 64);
            EVP_DigestFinal(ripemd160ctx, hash, NULL);

            // check
            // tmp = be64toh(*(unsigned long *)hash);
            tmp = *(unsigned long *)hash;
            // nice = (tmp == 0UL) ? 64 : __builtin_clzl(tmp);
            counter++;
        } while (tmp & 0x00000000000000ffUL);
    } while (tmp & 0x00000000ffffffffUL);
    tmp = be64toh(*(unsigned long *)hash);
    nice = (tmp == 0UL) ? 64 : __builtin_clzl(tmp);
    printf("%s, nice = %2d\n", passphrase, nice);
    EVP_MD_CTX_free(sha512ctx0);
    EVP_MD_CTX_free(sha512ctx1);
    EVP_MD_CTX_free(sha512ctx2);
    EVP_MD_CTX_free(ripemd160ctx);
    return NULL;
}

/**
 * @brief
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * ↓getDoubleで可能な限り1に近い値が出るseed
 * 155239116123415
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 * ハッシュの各バイトを１バイトにORで集約して結果が0xffにならなかったら成功
 * 丸数字の1から50までforで出す
 * timer_create+sigeventでタイマーを使って呼ばれたスレッドから新しくスレッドを起動する
 * TODO: CでSSLエンジンを使う
 * TODO: CでLibSSLなSSLエンジンを使う
 * TODO: javaでも直接SSLEngineを使ってみる
 * TODO: SocketChannel + SSLEngine + Selector
 * TODO: bitmessageをCで実装する、bitmessaged + bmctl の形式が良い
 * TODO: 新しいアドレスと鍵を動的にロードできないの、なんとかなりません？
 *
 * decodable random source?
 *
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int entrypoint(int argc, char **argv, char *const *envp)
{
    if (argc < 2)
    {
        return 1;
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    // EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    // EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
#endif

    const size_t threadsnum = strtoul(argv[1], NULL, 10) / 2;
    pid_t pid = 0;
    if ((pid = fork()) < 0)
    {
        perror("fork");
        return 1;
    }
    pthread_t threads[threadsnum];
    int ret = 0;
    for (size_t i = 0; i < threadsnum; i++)
    {
        ret = pthread_create(threads + i, NULL, task, NULL);
        if (ret != 0)
        {
            perror("ret");
            return 1;
        }
    }

    for (size_t i = 0; i < threadsnum; i++)
    {
        pthread_join(threads[i], NULL);
    }

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    if (pid == 0)
    {
        _exit(0);
    }
    else
    {
        wait(NULL);
    }
    return 0;
}
