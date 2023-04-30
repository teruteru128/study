
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
    if (argc < 2)
    {
        return 1;
    }
    uint64_t number = 0;
    size_t a = 0;
    uint64_t x = 0;
    uint64_t y = 0;
    unsigned char sigkey[65];
    unsigned char enckey[65];
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
#endif
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *md = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    do
    {
        a = getrandom(&number, 8, 0);
        x = (number >> 32) & 0xffffffffUL;
        y = (number >> 0) & 0xffffffffUL;
        load(sigkey, x);
        load(enckey, y);
        EVP_DigestInit_ex2(md, sha512, NULL);
        EVP_DigestUpdate(md, sigkey, 65);
        EVP_DigestUpdate(md, enckey, 65);
        EVP_DigestFinal_ex(md, hash, NULL);
        EVP_DigestInit_ex2(md, ripemd160, NULL);
        EVP_DigestUpdate(md, hash, 64);
        EVP_DigestFinal_ex(md, hash, NULL);
        if (hash[0] == 0)
        {
            for (size_t i = 0; i < 20; i++)
            {
                printf("%02x", hash[i]);
            }
            printf("\n");
            printf("%" PRIu64 ", %" PRIu64 ", %" PRIu64 "\n", number, x, y);
        }
    } while (hash[0] != 0);
    EVP_MD_CTX_free(md);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    return 0;
}
