
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
#include <liburing.h>
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
#if 0
    char wpath[PATH_MAX] = "";
    int prikeyfd = -1;
    int pubkeyfd = -1;
    int trimmedpubkeyfd = -1;
    FILE *prikeyf = NULL;
    FILE *pubkeyf = NULL;
    FILE *trimmedpubkeyf = NULL;
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    BN_CTX *ctx = NULL;
    BIGNUM *prikeybn = NULL;
    EC_POINT *pubkeyp = NULL;
    unsigned char *prikeymap = NULL;
    unsigned char *pubkeymap
        = mmap(NULL, 16777216 * 65, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pubkeymap == MAP_FAILED)
    {
        perror("mmap read");
        return 1;
    }
    unsigned char *trimmedpubkeymap
        = mmap(NULL, 16777216 * 64, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trimmedpubkeymap == MAP_FAILED)
    {
        perror("mmap write");
        return 1;
    }
    ssize_t d = 0;
    size_t j = 0;
    for (size_t i = 8; i < 256; i++)
    {
        // open private key file
        snprintf(rpath, PATH_MAX, "/mnt/d/keys/private/privateKeys%zu.bin", i);
        prikeyfd = open(rpath, O_RDONLY);
        if (prikeyfd < 0)
        {
            perror("open read");
            return 1;
        }
        // 秘密鍵ファイルをメモリーにマッピング
        prikeymap
            = mmap(NULL, 16777216 * 32, PROT_READ, MAP_PRIVATE, prikeyfd, 0);
        if (prikeymap == MAP_FAILED)
        {
            perror("mmap read");
            return 1;
        }
#pragma omp parallel default(none)                                            \
    shared(prikeymap, secp256k1, trimmedpubkeymap,                            \
           pubkeymap) private(ctx, j, prikeybn, pubkeyp)
        {
            ctx = BN_CTX_new();
            BN_CTX_start(ctx);
            prikeybn = BN_CTX_get(ctx);
            pubkeyp = EC_POINT_new(secp256k1);
#pragma omp for
            for (j = 0; j < 16777216; j++)
            {
                BN_bin2bn(prikeymap + j * 32, 32, prikeybn);
                EC_POINT_mul(secp256k1, pubkeyp, prikeybn, NULL, NULL, ctx);
                EC_POINT_point2oct(secp256k1, pubkeyp,
                                   POINT_CONVERSION_UNCOMPRESSED,
                                   pubkeymap + 65 * j, 65, ctx);
            }
            BN_CTX_end(ctx);
            BN_CTX_free(ctx);
            EC_POINT_free(pubkeyp);
        }
        for (j = 0; j < 16777216; j++)
        {
            memcpy(trimmedpubkeymap + 64 * j, pubkeymap + 65 * j + 1, 64);
        }
        snprintf(rpath, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin", i);
        pubkeyf = fopen(rpath, "wb");
        if (pubkeyf == NULL)
        {
            perror("fopen pubkeyf");
            return 1;
        }
        if (fwrite(pubkeymap, 65, 16777216, pubkeyf) < 16777216)
        {
            perror("write");
        }
        fclose(pubkeyf);
        snprintf(wpath, PATH_MAX,
                 "/mnt/d/keys/public/trimmed/publicKeys%zu.bin", i);
        trimmedpubkeyf = fopen(wpath, "wb");
        if (trimmedpubkeyf == NULL)
        {
            perror("fopen trimmedpubkeyf");
            return 1;
        }
        if (fwrite(trimmedpubkeymap, 64, 16777216UL, trimmedpubkeyf)
            < 16777216)
        {
            perror("write trimmedpubkeymap");
        }
        if (fclose(trimmedpubkeyf) == -1)
        {
            perror("close");
        }
        printf("%s done.\n", rpath);
    }
    munmap(pubkeymap, 16777216 * 65);
    munmap(trimmedpubkeymap, 16777216 * 64);
    EC_GROUP_free(secp256k1);
#else
    if (argc < 2)
    {
        return 1;
    }
    size_t a = 0x01ffffff;
    size_t b = 536870912;
    size_t needs = b - a;
    char rpath[PATH_MAX] = "";
    strncpy(rpath, argv[1], PATH_MAX);
    int fd = open(rpath, O_RDONLY);
    unsigned char *m
        = mmap(NULL, 16777216UL * 32, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    unsigned char mam[32];
    memset(mam, 0, 32);
    size_t l = argc >= 3 ? strtoul(argv[2], NULL, 0): 32;
    unsigned char *m2 = memmem(m, 16777216UL * 32, mam, l);
    if (m2 != NULL)
    {
        printf("0x%016lx, %ld\n", m2 - m, m2 - m);
    }
    munmap(m, 16777216 * 32);
#endif
    return 0;
}
