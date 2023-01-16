
#define _GNU_SOURCE 1
#define _DEFAULT_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "countdown.h"
#include "countdown2038.h"
#include "pngheaders.h"
#include "pngsample_gennoise.h"
#include "randomsample.h"
#include "roulette.h"
#include "searchAddressFromExistingKeys.h"
#include "timeutil.h"
#include <CL/opencl.h>
#include <bm.h>
#include <ctype.h>
#include <curl/curl.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <gmp.h>
#include <inttypes.h>
#include <java_random.h>
#include <jsonrpc-glib.h>
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
#include <printaddrinfo.h>
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
#include <sys/sysmacros.h>
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

#define IN                                                                    \
    "MEwDAgcAAgEgAiEA7Vo1+"                                                   \
    "Orf2xuuu6hTPAPldSfrUZZ7WYAzpRcO5DoYFLoCIF1JKVBctOGvMOy495O/"             \
    "BWFuFEYH4i1f6vU0b9+a64RD"

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
 *
 * decodable random source?
 *
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int hiho(int argc, char **argv, const char *const *envp)
{
    const EVP_MD *sha1 = EVP_sha1();
    EVP_MD_CTX *ctx[5];

    EVP_MD_CTX *ctx0 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx2 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx3 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx4 = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_DigestInit_ex2(ctx0, sha1, NULL);
    EVP_DigestInit_ex2(ctx1, sha1, NULL);
    EVP_DigestInit_ex2(ctx2, sha1, NULL);
    EVP_DigestInit_ex2(ctx3, sha1, NULL);
    EVP_DigestInit_ex2(ctx4, sha1, NULL);
#else
    EVP_DigestInit_ex(ctx0, sha1, NULL);
    EVP_DigestInit_ex(ctx1, sha1, NULL);
    EVP_DigestInit_ex(ctx2, sha1, NULL);
    EVP_DigestInit_ex(ctx3, sha1, NULL);
    EVP_DigestInit_ex(ctx4, sha1, NULL);
#endif
    EVP_DigestUpdate(ctx0, IN, strlen(IN));
    char buf[5] = "";
    unsigned char hash[20];
    size_t len = 0;
    size_t j = 0;
    size_t k = 0;
    size_t l = 0;
    size_t m = 0;
    double start = 0;
    double finish = 0;
    for (size_t i = 16; i < 17; i++)
    {
        EVP_MD_CTX_copy_ex(ctx1, ctx0);
        len = snprintf(buf, 5, "%zu", i);
        EVP_DigestUpdate(ctx1, buf, len);
        for (j = 0; j < 10000; j++)
        {
            start = omp_get_wtime();
            EVP_MD_CTX_copy_ex(ctx2, ctx1);
            len = snprintf(buf, 5, "%04zu", j);
            EVP_DigestUpdate(ctx2, buf, len);
            for (k = 0; k < 10000; k++)
            {
                EVP_MD_CTX_copy_ex(ctx3, ctx2);
                len = snprintf(buf, 5, "%04zu", k);
                EVP_DigestUpdate(ctx3, buf, len);
                for (l = 0; l < 10000; l++)
                {
                    EVP_MD_CTX_copy_ex(ctx4, ctx3);
                    len = snprintf(buf, 5, "%04zu", l);
                    EVP_DigestUpdate(ctx4, buf, len);
                    EVP_DigestFinal_ex(ctx4, hash, NULL);
#if BYTE_ORDER == LITTLE_ENDIAN
                    if (*(unsigned long *)hash & 0x00001fffffffffffUL)
#elif BYTE_ORDER == BIG_ENDIAN
                    if (*(unsigned long *)hash & 0xffffffffff1f0000UL)
#else
#error "unknown endian!"
#endif
                    {
                        continue;
                    }
                    printf("%zu%04zu%04zu%04zu, %d\n", i, j, k, l,
                           __builtin_ctzl(le64toh(*(unsigned long *)hash)));
                }
            }
            finish = omp_get_wtime();
            fprintf(stderr, "%zu%04zu00000000-%zu%04zu00000000 done(%lf)\n", i,
                    j, i, j + 1, finish - start);
        }
    }
    EVP_MD_CTX_free(ctx0);
    EVP_MD_CTX_free(ctx1);
    EVP_MD_CTX_free(ctx2);
    EVP_MD_CTX_free(ctx3);
    EVP_MD_CTX_free(ctx4);

    return 0;
}
