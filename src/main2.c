
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
#include <dirent.h>
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
    char inputpath[PATH_MAX];
    char outputpath[PATH_MAX];
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    unsigned char rawpubkey[65];
    FILE *outputf = NULL;
    int inputfd = -1;
    unsigned char *prikey = NULL;
    unsigned char *pubkey = mmap(NULL, 1090519040, PROT_READ | PROT_WRITE,
                                 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    for (size_t i = 9; i < 256; i++)
    {
        snprintf(inputpath, PATH_MAX, "/mnt/d/keys/private/privateKeys%zu.bin",
                 i);
        snprintf(outputpath, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin",
                 i);
        inputfd = open(inputpath, O_RDONLY);
        outputf = fopen(outputpath, "w");
        if (inputfd < 0 || outputf == NULL)
        {
            perror("open");
            break;
        }
        prikey = mmap(NULL, 536870912, PROT_READ, MAP_PRIVATE, inputfd, 0);
        close(inputfd);
        if (prikey == (void *)-1 || pubkey == (void *)-1)
        {
            perror("mmap");
            break;
        }
#pragma omp parallel default(none) shared(secp256k1, prikey, pubkey)
        {
            BN_CTX *ctx = BN_CTX_new();
            BN_CTX_start(ctx);
            BIGNUM *prikeybn = BN_CTX_get(ctx);
            EC_POINT *pubkeyp = EC_POINT_new(secp256k1);
#pragma omp for
            for (size_t j = 0; j < 16777216; j++)
            {
                BN_bin2bn(prikey + (j << 5), 32, prikeybn);
                EC_POINT_mul(secp256k1, pubkeyp, prikeybn, NULL, NULL, ctx);
                EC_POINT_point2oct(secp256k1, pubkeyp,
                                   POINT_CONVERSION_UNCOMPRESSED,
                                   pubkey + ((j << 6) + j), 65, ctx);
            }
            BN_CTX_end(ctx);
            BN_CTX_free(ctx);
            EC_POINT_free(pubkeyp);
        }
        fwrite(pubkey, 65, 16777216, outputf);
        munmap(prikey, 536870912);
        fclose(outputf);
        printf("%zu終わり\n", i);
        memset(inputpath, 0, PATH_MAX);
        memset(outputpath, 0, PATH_MAX);
    }
    munmap(pubkey, 1090519040);
    EC_GROUP_free(secp256k1);

    return 0;
}
