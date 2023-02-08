
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
    char rpath[PATH_MAX] = "";
    char wpath[PATH_MAX] = "";
    int rdfd = -1;
    int wrfd = -1;
    unsigned char *rdmap = NULL;
    unsigned char *wrmap = mmap(NULL, 16777216 * 64, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (wrmap == MAP_FAILED)
    {
        perror("mmap write");
        return 1;
    }
    ssize_t d = 0;
    for (size_t i = 0; i < 256; i++)
    {
        snprintf(rpath, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin", i);
        rdfd = open(rpath, O_RDONLY);
        if (rdfd < 0)
        {
            perror("open read");
            return 1;
        }
        rdmap = mmap(NULL, 16777216 * 65, PROT_READ, MAP_SHARED, rdfd, 0);
        if (rdmap == MAP_FAILED)
        {
            perror("mmap read");
            return 1;
        }
        close(rdfd);
        snprintf(wpath, PATH_MAX,
                 "/mnt/d/keys/public/trimmed/publicKeys%zu.bin", i);
        wrfd = open(wpath, O_WRONLY | O_CREAT, 0700);
        if (wrfd < 0)
        {
            perror("open write");
            return 1;
        }
        for (size_t j = 0; j < 16777216; j++)
        {
            memcpy(wrmap + 64 * i, rdmap + 65 * i + 1, 64);
        }
        munmap(rdmap, 1677716 * 65);
        if (write(wrfd, wrmap, 16777216UL * 64) < 0)
        {
            perror("write");
        }
        if (close(wrfd) == -1)
        {
            perror("close");
        }
        printf("%s done.\n", rpath);
    }
    munmap(wrmap, 16777216 * 64);

    return 0;
}
