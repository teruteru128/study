
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

/**
 * @brief open listened socket
 * オプションとかは仮引数で
 *
 * @return int listened socket
 */
static int openlistensocket()
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res = NULL;
    getaddrinfo(NULL, "8080", &hints, &res);
    int s = -1;
    for (struct addrinfo *ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        printaddrinfo(ptr);
        s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (s < 0)
        {
            s = -1;
            continue;
        }

        if (bind(s, ptr->ai_addr, ptr->ai_addrlen) != 0)
        {
            close(s);
            s = -1;
            continue;
        }

        if (listen(s, 16) != 0)
        {
            close(s);
            s = -1;
            continue;
        }
    }
    freeaddrinfo(res);
    return s;
}

static void *ewaitthread(void *arg) { return NULL; }
static void *downloadingthread(void *arg) { return NULL; }
static void *processdatathread(void *arg) {}

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
    int listensock = openlistensocket();
    printf("listen: %d\n", listensock);
    close(listensock);
    // create epoll fd
    // register listen socket to epoll fd
    // launch accept/signal receive thread
    /*
        if(listensocket == socket){
            register to epoll fd
        }else{
            send (push?/forward?) downloading thread
        }
     */
    // launch downloading thread
    // launch processing data thread
    return 0;
}
