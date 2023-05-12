
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

struct tmp
{
    unsigned long tmp;
    size_t size;
};

double getdouble(struct tmp *tmp)
{
    double out = 0;
    if (tmp->size >= 52)
    {
        out = (double)(((tmp->tmp) >> (52 - tmp->size)) & 0xfffffffffffffUL)
              / (1UL << 52);
        tmp->size -= 52;
        return out;
    }
    unsigned long work = 0;
    if (getrandom(&work, 7, 0) != 7)
    {
        return 1;
    }
    work = le64toh(work);
    tmp->tmp = (tmp->tmp << 4) | ((work >> 52) & 0xf);
    tmp->size += 4;
    out = (double)(work & 0xfffffffffffffUL) / (1UL << 52);
    return out;
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
    double complex point = 0 + 0 * I;
    double coefficient = 0;
    double radian = 0;
    struct tmp tmp = { 0 };
    double c = 0;
    double s = 0;
    for (size_t i = 0; i < 100; i++)
    {
        coefficient = getdouble(&tmp);
        radian = (2 * coefficient - 1) * M_PI;
        c = cos(radian);
        s = sin(radian);
        point += CMPLX(c, s);
        printf("%lf, %+lf pi, %+lf, %+lf, %+lf pi, %lf\n", coefficient,
               2 * coefficient - 1, creal(point), cimag(point), carg(point) / M_PI,
               cabs(point));
    }
    return 0;
}
