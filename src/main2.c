
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
    if (argc < 2)
    {
        fprintf(stderr, "path?\n");
        return 1;
    }
    // public key
    unsigned char target[64]
        = { 0x9c, 0x97, 0xfb, 0xdc, 0x9c, 0xad, 0x37, 0xfc, 0xdf, 0x93,
            0x2f, 0xc9, 0x0a, 0x49, 0x9d, 0x9d, 0x50, 0xc2, 0xe2, 0x5e, 0xa3,
            0xfc, 0x85, 0x03, 0x0d, 0x5d, 0x94, 0x24, 0x59, 0x5c, 0x91, 0xa8,
            0xc0, 0xd9, 0x9f, 0x9d, 0xe6, 0x30, 0x77, 0x02, 0xc8, 0x8b, 0x06,
            0x12, 0x55, 0x8a, 0x6d, 0x00, 0xef, 0xa9, 0xb4, 0xb3, 0x97, 0x16,
            0xac, 0xd7, 0x9b, 0xf6, 0x11, 0xe8, 0x02, 0xb0, 0x59, 0x22 };
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        return 1;
    }
    struct stat st;
    fstat(fd, &st);
    unsigned char *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED)
    {
        close(fd);
        return 1;
    }
    close(fd);
    unsigned char *p = memmem(map, st.st_size, target, 64);
    if (p != NULL)
    {
        printf("%ld(0x%016lx)\n", p - map, p - map);
    }
    else
    {
        printf("not found.\n");
    }

    munmap(map, st.st_size);
    return 0;
}
