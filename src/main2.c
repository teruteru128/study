
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

#define LOWER_THRESHOLD 1.0e-6
#define UPPER_BOUND 1.0e+4
#define INF 300

double complex zeta(double complex s)
{
    double complex outer_sum = 0;
    double complex inner_sum = 0;
    size_t j = 0;
    double complex c1 = 0;
    double complex c2 = 0;
    double complex c3 = 0;
    double complex prev = 1000000000;
    for (size_t m = 0; m <= INF; m++)
    {
        inner_sum = 0;
        for (j = 1; j <= m; j++)
        {
            c1 = ((j - 1) % 2 == 0) ? 1 : (-1);
            c2 = binomial(m - 1, j - 1);
            c3 = cpow(j, -s);
            inner_sum += c1 * c2 * c3;
        }
        // FIXME cpow(2, -m)が小さすぎて0になってしまうためinfになる
        inner_sum = inner_sum * cpow(2, -m) / (1 - cpow(2, 1 - s));

        outer_sum += inner_sum;

        if (cabs(prev - inner_sum) < LOWER_THRESHOLD)
        {
            break;
        }
        if (cabs(outer_sum) > UPPER_BOUND)
        {
            break;
        }
        prev = inner_sum;
    }

    return outer_sum;
}
#define WIDTH 1920
#define HEIGHT 1080

static size_t wordIndex(size_t bitIndex) { return bitIndex >> 3; }

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
    struct IHDR ihdr = { 0 };
    ihdr.width = WIDTH;
    ihdr.height = HEIGHT;
    ihdr.bit_depth = 8;
    ihdr.color_type = PNG_COLOR_TYPE_RGB;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_byte **data = malloc(sizeof(png_byte *) * HEIGHT);
    size_t x = 0;
    size_t y = 0;
    size_t width_length = (sizeof(png_byte) * WIDTH) * 3;
    for (y = 0; y < HEIGHT; y++)
    {
        data[y] = malloc(width_length);
        for (x = 0; x < width_length; x++)
        {
            data[y][x] = 0xff;
        }
    }
    size_t seed = 0;
    ssize_t len = getrandom(&seed, 6, 0);
    if (len < 6)
    {
        perror("getrandom");
        return 1;
    }
    seed = le64toh(seed);
    seed = initialScramble(seed);
    size_t numberOfVerticalLines = nextIntWithBounds(&seed, WIDTH / 4);
    size_t index = 0;
    size_t word = 0;
    png_color color;
    for (size_t i = 0; i < numberOfVerticalLines; i++)
    {
        x = nextIntWithBounds(&seed, WIDTH);
        len = getrandom(&color, sizeof(png_color), 0);
        if (len < sizeof(png_color))
        {
            perror("getrandom");
            return 1;
        }
        for (y = 0; y < HEIGHT; y++)
        {
            data[y][x * 3 + 0] = color.red;
            data[y][x * 3 + 1] = color.green;
            data[y][x * 3 + 2] = color.blue;
        }
    }
    size_t numberOfHorizontalLines = nextIntWithBounds(&seed, HEIGHT / 4);
    for (size_t i = 0; i < numberOfHorizontalLines; i++)
    {
        y = nextIntWithBounds(&seed, HEIGHT);
        len = getrandom(&color, sizeof(png_color), 0);
        if (len < sizeof(png_color))
        {
            perror("getrandom");
            return 1;
        }
        for (x = 0; x < width_length; x++)
        {
            data[y][x * 3 + 0] = color.red;
            data[y][x * 3 + 1] = color.green;
            data[y][x * 3 + 2] = color.blue;
        }
    }
    time_t cur = time(NULL);
    char f[FILENAME_MAX] = "";
    snprintf(f, FILENAME_MAX, "matrics-%ld.png", cur);
    write_png(f, &ihdr, NULL, NULL, 0, data);

    return 0;
}
