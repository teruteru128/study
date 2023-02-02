
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

/*
* Returns the size of the file whose open file descriptor is passed in.
* Properly handles regular file and block devices as well. Pretty.
* @see https://github.com/shuveb/io_uring-by-example/blob/58d05c1137d672337cf671e03e73bf7cc1b96342/03_cat_liburing/main.c#L22
* */
off_t get_file_size(int fd) {
    struct stat st;

    if(fstat(fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    if (S_ISBLK(st.st_mode)) {
        unsigned long long bytes;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            perror("ioctl");
            return -1;
        }
        return bytes;
    } else if (S_ISREG(st.st_mode))
        return st.st_size;

    return -1;
}

static volatile int running = 1;
static pthread_barrier_t barrier;

static void *func(void *arg)
{
    register uint_fast64_t d = 0;
    pthread_barrier_wait(&barrier);
    while (running)
    {
        d++;
    }
    return (void *)d;
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
    void *count = 0;
    pthread_t thread = 0;
    pthread_barrier_init(&barrier, NULL, 2);
    struct timespec spec;
    spec.tv_sec = 60;
    spec.tv_nsec = 0;
    pthread_create(&thread, NULL, func, NULL);
    pthread_barrier_wait(&barrier);
    nanosleep(&spec, NULL);
    running = 0;
    pthread_join(thread, &count);
    printf("%zu\n", (size_t)count);
    printf("%lf\n", (size_t)count / 60.);
    pthread_barrier_destroy(&barrier);
    return 0;
}
