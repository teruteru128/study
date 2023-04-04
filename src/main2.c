
#define _GNU_SOURCE 1
#define _DEFAULT_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

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
    if (strcmp(argv[1], "search") == 0)
    {
        char inputpath[PATH_MAX];
        int fd = -1;
        unsigned char *key = malloc(1090519040UL);
        unsigned char *current = NULL;
        for (size_t i = 0; i < 256; i++)
        {
            snprintf(inputpath, PATH_MAX,
                     "/mnt/d/keys/public/publicKeys%zu.bin", i);
            fd = open(inputpath, O_RDONLY);
            if (fd < 0)
            {
                continue;
            }
            read(fd, key, 1090519040UL);
            close(fd);
            for (size_t j = 0; j < 1090519040UL; j += 65)
            {
                current = key + j;
                if (current[1] == 0 && current[33] == 0
                    && ((current[2] == 0 && (current[34] & 0xf8) == 0)
                        || ((current[2] & 0xf8) == 0 && current[34] == 0)))
                {
                    printf("%zu, %zu: ", i, j / 65);
                    for (size_t k = 0; k < 65; k++)
                    {
                        printf("%02x", current[k]);
                    }
                    printf("\n");
                }
            }
            fprintf(stderr, "%s done.\n", inputpath);
        }
        return 0;
    }
    const char *filter = argv[1];

    size_t filterlength = strlen(filter);
    for (size_t i = 0; i < filterlength; i++)
    {
        if (!isdigit(filter[i]))
        {
            return 1;
        }
    }

    mpz_t num;
    mpz_init_set_ui(num, 1);

    char *str = NULL;

    for (int i = 0;; i++, mpz_mul_2exp(num, num, 1))
    {
        str = mpz_get_str(NULL, 10, num);
        if (strstr(str, filter) != NULL)
        {
            printf("%d, %lu\n", i, strlen(str));
            printf("%s\n", str);
            free(str);
            break;
        }
        free(str);
    }

    mpz_clear(num);
    return 0;
}
