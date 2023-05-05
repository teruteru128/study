
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
 *
 * @param key
 * @param index
 * @return int onerror: 0, onsuccess: 1
 */
int load(unsigned char *key, size_t index)
{
    size_t filenumber = (index >> 24) & 0xffUL;
    size_t fileoffset = ((index >> 0) & 0xffffffUL) * 65;
    char in[PATH_MAX] = "";
    snprintf(in, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin", filenumber);
    FILE *fin = fopen(in, "rb");
    if (fin == NULL)
    {
        return 0;
    }
    fseek(fin, SEEK_SET, fileoffset);
    size_t num = fread(key, 65, 1, fin);
    fclose(fin);
    return num == 1;
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
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *prikeybn = BN_CTX_get(ctx);
    EC_POINT *pubkeyp = EC_POINT_new(secp256k1);
    unsigned char *prikeybuf = malloc(32 * 256);
    unsigned char *pubkeybuf = malloc(65 * 256);
    unsigned char pubkeywork[65];
    FILE *prikeyf = NULL;
    FILE *pubkeyf = NULL;
    char path[PATH_MAX] = "";
    size_t j = 0;
    size_t k = 0;
    int success = 1;
    time_t start = 0;
    size_t a = 0;
    if (argc >= 2)
    {
        a = strtoul(argv[1], NULL, 10);
    }
    // 6まで終わり
    for (size_t i = a; i < 256; i++)
    {
        start = time(NULL);
        success = 1;
        snprintf(path, PATH_MAX, "/mnt/d/keys/private/privateKeys%zu.bin", i);
        prikeyf = fopen(path, "rb");
        snprintf(path, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin", i);
        pubkeyf = fopen(path, "rb");
        if (prikeyf == NULL || prikeyf == NULL)
        {
            perror("fopen");
            return 1;
        }
        for (j = 0; j < 65536; j++)
        {
            fread(prikeybuf, 32, 256, prikeyf);
            fread(pubkeybuf, 65, 256, pubkeyf);
            for (k = 0; k < 256; k++)
            {
                BN_bin2bn(prikeybuf + k * 32, 32, prikeybn);
                EC_POINT_mul(secp256k1, pubkeyp, prikeybn, NULL, NULL, ctx);
                EC_POINT_point2oct(secp256k1, pubkeyp,
                                   POINT_CONVERSION_UNCOMPRESSED, pubkeywork,
                                   65, ctx);
                if (memcmp(pubkeybuf + 65 * k, pubkeywork, 65) != 0)
                {
                    fprintf(stderr, "error!: %zu, %zu\n", i, j * 16 + k);
                }
            }
        }
        fclose(prikeyf);
        fclose(pubkeyf);
        if (success)
        {
            fprintf(stderr, "success!: %zu(%lf)\n", i,
                    difftime(time(NULL), start));
        }
        else
        {
            fprintf(stderr, "error!: %zu(%lf)\n", i,
                    difftime(time(NULL), start));
        }
    }
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    EC_GROUP_free(secp256k1);
    free(prikeybuf);
    free(pubkeybuf);
    return 0;
}
