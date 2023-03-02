
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

static volatile sig_atomic_t re = 1;

void handler(int h, siginfo_t *a, void *v) { re ^= 1; }

struct ServerConfig
{
    int socket;
};

static struct ServerConfig config = { 0 };

void *func(void *arg)
{
    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct timespec req, rem;
    req.tv_sec = 60;
    req.tv_nsec = 0;
    int t = 0;
    char errbuf[512];
    char *r = NULL;
    while (re)
    {
        errno = 0;
        if (getaddrinfo("www.pixiv.net", "https", &hints, &res))
        {
            fprintf(stderr, "getaddrinfo: %s(%ld)\n",
                    strerror_r(errno, errbuf, 512), time(NULL));
        }
        else
        {
            freeaddrinfo(res);
        }
        t = nanosleep(&req, &rem);
        if (t != 0)
        {
            fprintf(stderr, "t is %d(%ld.%09ld), re is %d\n", t, rem.tv_sec,
                    rem.tv_nsec, re);
        }
    }
    if (re == 0)
    {
        fprintf(stderr, "re is 0\n");
    }
    return NULL;
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
int hiho(int argc, char **argv, char *const *envp)
{
    struct sigaction action = { 0 };
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &action, NULL) < 0)
    {
        perror("sigaction");
        return 1;
    }

    pthread_t thread = 0;
    int r = pthread_create(&thread, NULL, func, NULL);
    if (r != 0)
    {
        return 1;
    }
    fputs("chottomattene...\n", stderr);
    pthread_join(thread, NULL);

    int e = epoll_create1(0);
    if (e < 0)
    {
        perror("epoll_create");
    }

    return 0;
}
