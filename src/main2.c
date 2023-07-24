
#define _DEFAULT_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

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

#define SWAP64(n)                                                             \
    (((n) << 56) | (((n)&0xff00) << 40) | (((n)&0xff0000) << 24)              \
     | (((n)&0xff000000) << 8) | (((n) >> 8) & 0xff000000)                    \
     | (((n) >> 24) & 0xff0000) | (((n) >> 40) & 0xff00) | ((n) >> 56))
/*
nonlinear functions at bit level: exor, mux, -, mux, -
*/
unsigned int f(int j, unsigned int x, unsigned int y, unsigned int z)
{
    switch (j >> 4)
    {
    case 0:
        return x ^ y ^ z;
    case 1:
        return (x & y) | (!x & z);
    case 2:
        return (x | y) ^ z;
    case 3:
        return (x & z) | (y & !z);
    default:
        return x ^ (y | !z);
    }
}

/*
added constants (hexadecimal)
*/
static unsigned int K[]
    = { 0x00000000, 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xA953FD4E };
static unsigned int K_d[]
    = { 0x50A28BE6, 0x5C4DD124, 0x6D703EF3, 0x7A6D76E9, 0x00000000 };

/*
selection of message word
r(j) = j
r(16..31) = 7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8
r(32..47) = 3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12
r(48..63) = 1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2
r(64..79) = 4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
r'( 0..15) = 5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12
r'(16..31) = 6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2
r'(32..47) = 15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13
r'(48..63) = 8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14
r'(64..79) = 12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
*/
static const int r[]
    = { 0, 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        7, 4,  13, 1,  10, 6,  15, 3,  12, 0, 9,  5,  2,  14, 11, 8,
        3, 10, 14, 4,  9,  15, 8,  1,  2,  7, 0,  6,  13, 11, 5,  12,
        1, 9,  11, 10, 0,  8,  12, 4,  13, 3, 7,  15, 14, 5,  6,  2,
        4, 0,  5,  9,  7,  12, 2,  10, 14, 1, 3,  8,  11, 6,  15, 13 };
static const int r_d[]
    = { 5,  14, 7,  0, 9, 2,  11, 4,  13, 6,  15, 8,  1,  10, 3,  12,
        6,  11, 3,  7, 0, 13, 5,  10, 14, 15, 8,  12, 4,  9,  1,  2,
        15, 5,  1,  3, 7, 14, 6,  9,  11, 8,  12, 2,  10, 0,  4,  13,
        8,  6,  4,  1, 3, 11, 15, 0,  5,  12, 2,  13, 9,  7,  10, 14,
        12, 15, 10, 4, 1, 5,  8,  7,  6,  2,  13, 14, 0,  3,  9,  11 };
/*
amount for rotate left (rol)
s( 0..15) = 11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8
s(16..31) = 7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12
s(32..47) = 11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5
s(48..63) = 11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12
s(64..79) = 9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
s'( 0..15) = 8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6
s'(16..31) = 9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11
s'(32..47) = 9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5
s'(48..63) = 15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8
s'(64..79) = 8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
*/
const int s[]
    = { 11, 14, 15, 12, 5,  8,  7,  9,  11, 13, 14, 15, 6,  7,  9,  8,
        7,  6,  8,  13, 11, 9,  7,  15, 7,  12, 15, 9,  11, 7,  13, 12,
        11, 13, 6,  7,  14, 9,  13, 15, 14, 8,  13, 6,  5,  12, 7,  5,
        11, 12, 14, 15, 14, 15, 9,  8,  9,  14, 5,  6,  8,  6,  5,  12,
        9,  15, 5,  11, 6,  8,  13, 12, 5,  12, 13, 14, 11, 8,  5,  6 };
const int s_d[]
    = { 8,  9,  9,  11, 13, 15, 15, 5,  7,  7,  8,  11, 14, 14, 12, 6,
        9,  13, 15, 7,  12, 8,  9,  11, 7,  7,  12, 7,  6,  15, 13, 11,
        9,  7,  15, 11, 8,  6,  6,  14, 12, 13, 5,  14, 13, 13, 7,  5,
        15, 5,  8,  11, 14, 14, 6,  14, 6,  9,  12, 9,  12, 5,  15, 8,
        8,  5,  12, 9,  12, 5,  14, 6,  8,  13, 6,  5,  15, 13, 11, 11 };

typedef struct
{
    unsigned int buffer[32];
    uint32_t length;
} ripemd160_ctx;

#define PUTCHAR(buf, index, val)                                              \
    (buf)[(index) >> 2]                                                       \
        = ((buf)[(index) >> 2] & ~(0xffU << (((index)&3) << 3)))              \
          + ((val) << (((index)&3) << 3))

void ripemd160_init(ripemd160_ctx *ctx, unsigned char *p, unsigned char len)
{
    uint32_t *b32 = ctx->buffer;
    for (int i = 0; i < len; i++)
    {
        PUTCHAR(b32, i, p[i]);
    }
    ctx->length = len;

    // append 1 to ctx buffer
    uint32_t length = ctx->length;
    PUTCHAR(b32, length, 0x80);
    // byte-unit padding
    while ((++length & 3) != 0)
    {
        PUTCHAR(b32, length, 0);
    }

    uint32_t *buffer32 = b32 + (length >> 2);
    // int-unit padding
    // append 0 to 128
    for (uint32_t i = length; i < 120; i += 4)
    {
        *buffer32++ = 0;
    }
    // append length to buffer
    uint64_t *buffer64 = (uint64_t *)ctx->buffer;
    buffer64[15] = SWAP64(((uint64_t)ctx->length) * 8);
}

#define H0 0x67452301
#define H1 0xEFCDAB89
#define H2 0x98BADCFE
#define H3 0x10325476
#define H4 0xC3D2E1F0

unsigned int rol(unsigned int x, int si)
{
    return (x << si) | (x >> (32 - si));
}

void ripemd160(unsigned char *hash, unsigned char *x)
{
    ripemd160_ctx ctx;
    ripemd160_init(&ctx, x, 64);

    uint32_t *data = ctx.buffer;
    int i = 0;
    int j = 0;
    unsigned int h0 = H0;
    unsigned int h1 = H1;
    unsigned int h2 = H2;
    unsigned int h3 = H3;
    unsigned int h4 = H4;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int a_d;
    unsigned int b_d;
    unsigned int c_d;
    unsigned int d_d;
    unsigned int e_d;
    unsigned int t;
    for (i = 0; i < 2; i++)
    {
        a = a_d = h0;
        b = b_d = h1;
        c = c_d = h2;
        d = d_d = h3;
        e = e_d = h4;
        for (j = 0; j < 80; j++)
        {
            t = rol(a + f(j, b, c, d) + data[(i << 4) + r[j]] + K[j >> 4],
                    s[j])
                + e;
            a = e;
            e = d;
            d = rol(c, 10);
            c = b;
            b = t;
            t = rol(a_d + f(79 - j, b_d, c_d, d_d) + data[(i << 4) + r_d[j]]
                        + K_d[j >> 4],
                    s_d[j])
                + e_d;
            a_d = e_d;
            e_d = d_d;
            d_d = rol(c_d, 10);
            c_d = b_d;
            b_d = t;
        }
        t = h1 + c + d_d;
        h1 = h2 + d + e_d;
        h2 = h3 + e + a_d;
        h3 = h4 + a + b_d;
        h4 = h0 + b + c_d;
        h0 = t;
    }
    *((unsigned int *)hash + 0) = h0;
    *((unsigned int *)hash + 1) = h1;
    *((unsigned int *)hash + 2) = h2;
    *((unsigned int *)hash + 3) = h3;
    *((unsigned int *)hash + 4) = h4;
}

// https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
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
int entrypoint(int argc, char **argv, char *const *envp)
{
    unsigned char hash[20];
    unsigned char x[64] = { 0 };
    ripemd160(hash, x);
    for (size_t i = 0; i < 20; i++)
    {
        printf("%02x", hash[i]);
    }
    printf("\n");
    return 0;
}
