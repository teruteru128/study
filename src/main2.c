
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
#include <iconv.h>
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
#include <openssl/ssl.h>
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

#define errchk(v, f)                                                          \
    if (!v)                                                                   \
    {                                                                         \
        unsigned long err = ERR_get_error();                                  \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL));           \
        return EXIT_FAILURE;                                                  \
    }

#define PREFIX_LENGTH 56

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
int hiho(int argc, char **argv, char *const *envp)
{
    unsigned char prefix[PREFIX_LENGTH];
    if (getrandom(prefix, PREFIX_LENGTH, 0) != PREFIX_LENGTH)
    {
        return 1;
    }
    EVP_MD_CTX *ctx1 = EVP_MD_CTX_new();
    if (ctx1 == NULL)
    {
        return 1;
    }
    const EVP_MD *sha512 = EVP_sha512();
    if (sha512 == NULL)
    {
        return 1;
    }
    errchk(EVP_DigestInit_ex2(ctx1, sha512, NULL), EVP_DigestInit_ex2);
    errchk(EVP_DigestUpdate(ctx1, prefix, PREFIX_LENGTH), EVP_DigestUpdate);
    volatile atomic_int r = 1;
#pragma omp parrel default(none) shared(ctx1, prefix, sha512, r)
    {
        EVP_MD_CTX *ctx2 = EVP_MD_CTX_new();
        if (ctx2 == NULL)
        {
            return 1;
        }
        errchk(EVP_DigestInit_ex2(ctx2, sha512, NULL), EVP_DigestInit_ex2);
        unsigned long suffix = 0;
        unsigned char hash[EVP_MAX_MD_SIZE];
#pragma omp for
        for(unsigned long counter = 0; counter < 0xFFFFFFFFFFFFFFFFUL; counter++)
        {
            suffix = htobe64(counter);
            EVP_MD_CTX_copy_ex(ctx2, ctx1);
            EVP_DigestUpdate(ctx2, &suffix, 8);
            EVP_DigestFinal_ex(ctx2, hash, NULL);
            if ((*((unsigned long *)hash)) & 0x00000000ffffffUL)
            {
                continue;
            }
#pragma omp critical
            {
                for (size_t i = 0; i < 64; i++)
                    printf("%02x", hash[i]);
                printf(":");
                for (size_t i = 0; i < PREFIX_LENGTH; i++)
                    printf("%02x", prefix[i]);
                printf("%016lx", counter);
                printf("\n");
            }
        }
    }
    EVP_MD_CTX_free(ctx1);
    return 0;
}
