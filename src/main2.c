
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

#define IN                                                                    \
    "MEwDAgcAAgEgAiEA7Vo1+"                                                   \
    "Orf2xuuu6hTPAPldSfrUZZ7WYAzpRcO5DoYFLoCIF1JKVBctOGvMOy495O/"             \
    "BWFuFEYH4i1f6vU0b9+a64RD"

int gettimebaserandom(unsigned char *a, size_t b)
{
    if (a == NULL || b == 0)
    {
        return 1;
    }
    struct timespec spec;
    for (size_t i = 0; i < b; i++)
    {
        clock_gettime(CLOCK_REALTIME, &spec);
        a[i] = spec.tv_nsec & 0xff;
    }
    return 0;
}

static volatile int running = 1;
static pthread_barrier_t barrier;

static void *func(void *arg)
{
    if (arg == NULL)
    {
        return NULL;
    }
    size_t *count = (size_t *)arg;
    pthread_barrier_wait(&barrier);
    register size_t d = 0;
    while (running)
    {
        d++;
    }
    *count = d;
    return arg;
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
    size_t count = 0;
    pthread_t thread = 0;
    pthread_barrier_init(&barrier, NULL, 2);
    struct timespec spec;
    spec.tv_sec = 1;
    spec.tv_nsec = 0;
    pthread_create(&thread, NULL, func, &count);
    pthread_barrier_wait(&barrier);
    nanosleep(&spec, NULL);
    running = 0;
    pthread_join(thread, NULL);
    pthread_barrier_destroy(&barrier);
    printf("%zu\n", count);

    GError *error = NULL;
    gboolean ret = FALSE;

    g_autoptr(GSocketClient) sclient = g_socket_client_new();
    g_autoptr(GSocketConnection) connection = g_socket_client_connect_to_host(
        sclient, "192.168.12.8:8442", 0, NULL, &error);
    if (connection == NULL || error != NULL)
    {
        g_warning("g_socket_client_connect_to_host: %s", error->message);
        g_error_free(error);
    }
    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add(builder, "i", 111);
    g_variant_builder_add(builder, "i", 222);
    g_autoptr(GVariant) array = g_variant_builder_end(builder);
    g_autoptr(GVariant) return_value = NULL;
    g_autoptr(JsonrpcClient) client
        = jsonrpc_client_new(G_IO_STREAM(connection));
    ret = jsonrpc_client_call(client, "add", array, NULL, &return_value,
                              &error);
    if (return_value == NULL || ret != TRUE)
    {
        g_warning("jsonrpc_client_call: %s", error->message);
        g_error_free(error);
        return 1;
    }
    ret = jsonrpc_client_close(client, NULL, &error);
    if (!ret)
    {
        g_warning("jsonrpc_client_close: %s", error->message);
        g_error_free(error);
        return 1;
    }
    return 0;
}
