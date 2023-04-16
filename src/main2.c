
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

    size_t capacity = 0;
    // null終端されていない文字列データ
    char *buffer = NULL;
    char *tmp = NULL;
    char inbuf[BUFSIZ];
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    ssize_t r = 0;
    while (1)
    {
        r = read(fd, inbuf, BUFSIZ);
        if (r < 0)
        {
            return 1;
        }
        capacity += r;
        tmp = realloc(buffer, capacity);
        if (tmp == NULL)
        {
            perror("realloc");
            exit(1);
        }
        buffer = tmp;
        memcpy(buffer + (capacity - r), inbuf, r);
        ssize_t start = capacity - r - 1;
        if (start < 0)
        {
            start = 0;
        }
        for (ssize_t i = start; i < capacity; i++)
        {
            if (buffer[i] == 0x0d && buffer[i + 1] == 0x0a)
            {
                char *line = malloc(i + 1);
                memcpy(line, buffer, i);
                // terminate txt
                line[i] = 0;

                // ここでコンテキストと行データをセットにして別スレッドへ転送

                // truncate
                size_t newBufferCapacity = capacity - i - 2;
                memmove(buffer, buffer + i + 2, newBufferCapacity);
                char *newBuffer = realloc(buffer, newBufferCapacity);
                if (newBuffer == NULL)
                {
                    perror("realloc(truncate)");
                    exit(1);
                }
                buffer = newBuffer;
            }
        }
    }
    return 0;
}
