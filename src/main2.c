
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

#define NUM 360

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
 * TODO CでSSLエンジンを使う
 * TODO CでLibSSLなSSLエンジンを使う
 * TODO javaでも直接SSLEngineを使ってみる
 * TODO SocketChannel + SSLEngine + Selector
 * TODO bitmessageをCで実装する、bitmessaged + bmctl の形式が良い
 * TODO
 * PyBitmessageは新しいアドレスと鍵を動的にロードできないの、なんとかなりません？
 * TODO EPSPで１行の最大長さがわからないのなんとかなりませんか？
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
    size_t snum = 0;
    if (argc >= 2)
    {
        snum = strtoul(argv[1], NULL, 10);
    }
    size_t y;
    size_t yy;
    size_t yyy;
    size_t x;
    size_t xx;
    size_t xxx;
    size_t ii;
    unsigned char *key = malloc(65 * (1UL << 24));
    FILE *fd = fopen("/mnt/d/keys/public/publicKeys0.bin", "rb");
    if (fd == NULL)
    {
        perror("fopen");
        return 1;
    }
    size_t num = fread(key, 65, 1 << 24, fd);
    if (num < 16777216)
    {
        perror("fread");
        fclose(fd);
        return 1;
    }
    fclose(fd);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
#endif
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    time_t start = 0;
    for (size_t i = snum; i < 65536; i++)
    {
        y = (i >> 8) & 0xff;
        x = (i >> 0) & 0xff;
        start = time(NULL);
#pragma omp parallel default(none) private(xx, yy, xxx, yyy)                  \
    shared(sha512, ripemd160, x, y, key)
        {
            EVP_MD_CTX *sha512ctx1 = EVP_MD_CTX_new();
            EVP_MD_CTX *sha512ctx2 = EVP_MD_CTX_new();
            EVP_MD_CTX *ripemd160ctx1 = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
            EVP_DigestInit_ex2(sha512ctx2, sha512, NULL);
#else
            EVP_DigestInit_ex(sha512ctx2, sha512, NULL);
#endif
            unsigned char hash[EVP_MAX_MD_SIZE];
            uint64_t leading64 = 0;
            size_t offset_x = 0;
            size_t offset_y = 0;
#pragma omp for
            for (yy = 0; yy < 256; yy++)
            {
                for (xx = 0; xx < 256; xx++)
                {
                    offset_y = (y << 16) + (yy << 8);
                    for (yyy = 0; yyy < 256; yyy++, offset_y++)
                    {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
                        EVP_DigestInit_ex2(sha512ctx1, sha512, NULL);
#else
                        EVP_DigestInit_ex(sha512ctx1, sha512, NULL);
#endif
                        EVP_DigestUpdate(sha512ctx1,
                                         key + (offset_y << 6) + offset_y, 65);
                        offset_x = (x << 16) + (xx << 8);
                        for (xxx = 0; xxx < 256; xxx++, offset_x++)
                        {
                            EVP_MD_CTX_copy_ex(sha512ctx2, sha512ctx1);
                            EVP_DigestUpdate(sha512ctx1,
                                             key + (offset_x << 6) + offset_x,
                                             65);
                            EVP_DigestFinal_ex(sha512ctx2, hash, NULL);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
                            EVP_DigestInit_ex2(ripemd160ctx1, ripemd160, NULL);
#else
                            EVP_DigestInit_ex(ripemd160ctx1, ripemd160, NULL);
#endif
                            EVP_DigestUpdate(ripemd160ctx1, hash, 64);
                            EVP_DigestFinal_ex(ripemd160ctx1, hash, NULL);
                            leading64 = htobe64(*(unsigned long *)hash);
                            if (leading64 & 0xffffffffff000000UL)
                            {
                                continue;
                            }
#pragma omp critical
                            {
                                size_t count = (leading64 == 0)
                                                   ? 64
                                                   : __builtin_clzl(leading64);
                                printf("%zu: %zu, %zu\n", count, offset_y,
                                       offset_x);
                            }
                        }
                    }
                }
            }
            EVP_MD_CTX_free(sha512ctx1);
            EVP_MD_CTX_free(sha512ctx2);
            EVP_MD_CTX_free(ripemd160ctx1);
        }
        fprintf(stderr, "%zu/65536 done, %ldseconds\n", x + 1,
                time(NULL) - start);
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    return 0;
}
