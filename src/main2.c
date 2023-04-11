
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

#define LOWER_THRESHOLD 1.0e-6
#define UPPER_BOUND 1.0e+4
#define INF 300

double complex zeta(double complex s)
{
    double complex outer_sum = 0;
    double complex inner_sum = 0;
    size_t j = 0;
    double complex c1 = 0;
    double complex c2 = 0;
    double complex c3 = 0;
    double complex prev = 1000000000;
    for (size_t m = 0; m <= INF; m++)
    {
        inner_sum = 0;
        for (j = 1; j <= m; j++)
        {
            c1 = ((j - 1) % 2 == 0) ? 1 : (-1);
            c2 = binomial(m - 1, j - 1);
            c3 = cpow(j, -s);
            inner_sum += c1 * c2 * c3;
        }
        // FIXME cpow(2, -m)が小さすぎて0になってしまうためinfになる
        inner_sum = inner_sum * cpow(2, -m) / (1 - cpow(2, 1 - s));

        outer_sum += inner_sum;

        if (cabs(prev - inner_sum) < LOWER_THRESHOLD)
        {
            break;
        }
        if (cabs(outer_sum) > UPPER_BOUND)
        {
            break;
        }
        prev = inner_sum;
    }

    return outer_sum;
}
#define WIDTH 1920
#define HEIGHT 1080

static size_t wordIndex(size_t bitIndex) { return bitIndex >> 3; }

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
    /*
    uint64_t current = 0;
    uint64_t min = UINT64_MAX;
    struct timespec spec = { 0 };
    spec.tv_sec = 0;
    spec.tv_nsec = 800000000UL;
    do
    {
        getrandom(&current, sizeof(uint64_t), 0);
        if (current < min)
        {
            printf("%zu->%zu\n", min, current);
            min = current;
        }
        if (current != 0)
        {
            nanosleep(&spec, NULL);
        }
    } while (current != 0);
    */
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    // EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    // EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
#endif
    uuid_t uuid;
    char passphrase[UUID_STR_LEN] = "";
    long signingKeyNonce = 0;
    long encryptionKeyNonce = 1;
    unsigned char nonce[2] = { 0x00, 0x01 };
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
    EVP_MD_CTX *sha512ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *sha512ctx2 = EVP_MD_CTX_new();
    EVP_MD_CTX *ripemd160ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_DigestInit_ex2(sha512ctx2, sha512, NULL);
#else
    EVP_DigestInit_ex(sha512ctx1, sha512, NULL);
#endif
    do
    {
        // passphrase generate
        uuid_generate_random(uuid);
        uuid_unparse_lower(uuid, passphrase);

        // private keys generate
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(sha512ctx1, sha512, NULL);
#else
        EVP_DigestInit_ex(sha512ctx1, sha512, NULL);
#endif
        EVP_DigestUpdate(sha512ctx1, passphrase, 36);
        EVP_MD_CTX_copy_ex(sha512ctx2, sha512ctx1);

        EVP_DigestUpdate(sha512ctx1, nonce, 1);
        EVP_DigestFinal_ex(sha512ctx1, potentialPrivSigningKey, NULL);

        EVP_DigestUpdate(sha512ctx2, nonce1, 1);
        EVP_DigestFinal_ex(sha512ctx2, potentialPrivEncryptionKey, NULL);

        // public keys generate
        BN_bin2bn(potentialPrivSigningKey, 32, prikey);
        EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, bnctx);
        EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED,
                           potentialPubSigningKey, 65, bnctx);

        BN_bin2bn(potentialPrivEncryptionKey, 32, prikey);
        EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, bnctx);
        EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED,
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
        tmp = be64toh(*(unsigned long *)hash);
        nice = tmp == 0UL ? 64 : __builtin_clzl(tmp);
    } while (nice < 24);
    printf("%s, nice = %d\n", passphrase, nice);
    EVP_MD_CTX_free(sha512ctx2);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    return 0;
}
