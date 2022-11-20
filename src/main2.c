
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

#define SECKEY0 "ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc="
#define SECKEY1 "cm2E2vmE0Nd8aVP/4Ph2S1R6C5bkC1H7CiUBzbcDG3U="
#define SECKEY2 "BixgbLYk35GP+XHYdK/DgSWIUXyCTwCwEtY4h/G22dw="
#define SECKEY3 "BH4RDmdo0Yq0Ftiw0lm9ej5BmpZ35kEw2kaWZlZ0Do8="
#define SECKEY4 "lMhxDh6RPpWOsnJMeS12pTJ/j7EPn+ugpdbNQCGbiwc="
#define SECKEY5 "9hZn+KDlwjgrbyFpaX5ibuaO4QfaFbIL79NUrwJlcRQ="
#define SECKEY6                                                               \
    "T+tDF4I700WFkFhGieYxWgQKPO/MDcntDYzMrqQSZjzwV2DzaI1OM/"                  \
    "CsJWE30WBqMI1SxbEQHufR1A76I7ayWN=="
#define SECKEY7                                                               \
    "nySkaCQxGErccmiqLznSQduXgFICpjnl2bo7n3FAhQMlku79plIeL85/"                \
    "etpN865GAnlUpErSppEYHvn4couGh3=="
#define SECKEY8                                                               \
    "ns2bQQ4zlnfcCTSAxEH3gDDYHcBswKw92jQeEgm+9tse74XdX+LNwgfw7OsMUjOGtLMb7R/" \
    "kXNRXYv1AHi71iV=="
#define SECKEY9                                                               \
    "NxhJ5JwWhUtUccCfJNtVqzdpCMGOaAtknmcEKLyglZFNXE66EiFi9wPFekwekx3ln8m9v5w" \
    "nfv7V8jSrpZ/SHQ=="
#define SECKEY10                                                              \
    "+3n5qDbtpicXBy+Yyol/TJkg2IoQ01vZ/U2SvgpP+Fdm4DrIYngY7X0ZS53rc/KKIHT//"   \
    "jVqNwNBz1sGFyYUDg=="
#define SECKEY11                                                              \
    "cLtHGFI7X/"                                                              \
    "Xl6Ly03DczMzl2bsHJmI2BMQKKCckUek5vTIiltDPfT3PxdT6zxW1LzwVqJIsQEkxxPNTsw" \
    "gpSFg=="
#define SECKEY12                                                              \
    "pMQBNF+F12AXT3T0mQq7S0l1VcCr/Dw2Q54zeuHH0/1ExLgbhHEsmAHf3WR9nK/Ku1Mc/"   \
    "eU3vaAO78yplJB76A=="
#define SECKEY13                                                              \
    "QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUF" \
    "BQUFBQUFBQUFBQQ=="
#define SECKEY14                                                              \
    "D8BH6DLNJekZ5jiiIVSnyS5ziE9XJSRG5bA9OdiFdjee6HTxHxFQXyEQdhfN+"           \
    "E69RKToLYXGDxK2X9v9eEcbUxdSp9tbptXegxkNQgIxg97BAq9gtmxPm4Ebngl/Q/I4"
#define SECKEY15                                                              \
    "cLJlMSoCYBgR0d/"                                                         \
    "bg7zG1B77BBWy7f1KLiJG5b8mPmlD8dAJKCZSEFRdWLuxSyRjgFFeiMm4+l+2SNIhL/"     \
    "SBma7ABhg232DeJkbUcZJKqBfAI9taPQ5Y9bwIXrcjxqMx"

char *clonekey(int index)
{
    if (index < 0 || 16 <= index)
    {
        return NULL;
    }
    char *key = NULL;
    switch (index)
    {
    case 0:
        key = strdup(SECKEY0);
        break;
    case 1:
        key = strdup(SECKEY1);
        break;
    case 2:
        key = strdup(SECKEY2);
        break;
    case 3:
        key = strdup(SECKEY3);
        break;
    case 4:
        key = strdup(SECKEY4);
        break;
    case 5:
        key = strdup(SECKEY5);
        break;
    case 6:
        key = strdup(SECKEY6);
        break;
    case 7:
        key = strdup(SECKEY7);
        break;
    case 8:
        key = strdup(SECKEY8);
        break;
    case 9:
        key = strdup(SECKEY9);
        break;
    case 10:
        key = strdup(SECKEY10);
        break;
    case 11:
        key = strdup(SECKEY11);
        break;
    case 12:
        key = strdup(SECKEY12);
        break;
    case 13:
        key = strdup(SECKEY13);
        break;
    case 14:
        key = strdup(SECKEY14);
        break;
    case 15:
        key = strdup(SECKEY15);
        break;
    default:
        break;
    }
    return key;
}

int countdownb(int argc, char **argv)
{
    struct tm tm = { 0 };
    tm.tm_year = 0;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    for (size_t i = 0; i < 6 && i + 1 < argc; i++)
    {
        switch (i)
        {
        case 0:
            tm.tm_year = strtol(argv[i + 1], NULL, 10) - 1900;
            break;
        case 1:
            tm.tm_mon = strtol(argv[i + 1], NULL, 10) - 1;
            break;
        case 2:
            tm.tm_mday = MIN(MAX(strtol(argv[i + 1], NULL, 10), 1), 31);
            break;
        case 3:
            tm.tm_hour = strtol(argv[i + 1], NULL, 10);
            break;
        case 4:
            tm.tm_min = strtol(argv[i + 1], NULL, 10);
            break;
        case 5:
            tm.tm_sec = strtol(argv[i + 1], NULL, 10);
            break;

        default:
            break;
        }
    }

    countdowns(&tm);
    return 0;
}

/**
 * @brief
 * 秘密鍵かな？
 * ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc=
 * cm2E2vmE0Nd8aVP/4Ph2S1R6C5bkC1H7CiUBzbcDG3U=
 * BixgbLYk35GP+XHYdK/DgSWIUXyCTwCwEtY4h/G22dw=
 * BH4RDmdo0Yq0Ftiw0lm9ej5BmpZ35kEw2kaWZlZ0Do8=
 * lMhxDh6RPpWOsnJMeS12pTJ/j7EPn+ugpdbNQCGbiwc=
 * 9hZn+KDlwjgrbyFpaX5ibuaO4QfaFbIL79NUrwJlcRQ=
 * T+tDF4I700WFkFhGieYxWgQKPO/MDcntDYzMrqQSZjzwV2DzaI1OM/CsJWE30WBqMI1SxbEQHufR1A76I7ayWN==
 * nySkaCQxGErccmiqLznSQduXgFICpjnl2bo7n3FAhQMlku79plIeL85/etpN865GAnlUpErSppEYHvn4couGh3==
 * ns2bQQ4zlnfcCTSAxEH3gDDYHcBswKw92jQeEgm+9tse74XdX+LNwgfw7OsMUjOGtLMb7R/kXNRXYv1AHi71iV==
 * NxhJ5JwWhUtUccCfJNtVqzdpCMGOaAtknmcEKLyglZFNXE66EiFi9wPFekwekx3ln8m9v5wnfv7V8jSrpZ/SHQ==
 * +3n5qDbtpicXBy+Yyol/TJkg2IoQ01vZ/U2SvgpP+Fdm4DrIYngY7X0ZS53rc/KKIHT//jVqNwNBz1sGFyYUDg==
 * cLtHGFI7X/Xl6Ly03DczMzl2bsHJmI2BMQKKCckUek5vTIiltDPfT3PxdT6zxW1LzwVqJIsQEkxxPNTswgpSFg==
 * pMQBNF+F12AXT3T0mQq7S0l1VcCr/Dw2Q54zeuHH0/1ExLgbhHEsmAHf3WR9nK/Ku1Mc/eU3vaAO78yplJB76A==
 * QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQQ==
 * D8BH6DLNJekZ5jiiIVSnyS5ziE9XJSRG5bA9OdiFdjee6HTxHxFQXyEQdhfN+E69RKToLYXGDxK2X9v9eEcbUxdSp9tbptXegxkNQgIxg97BAq9gtmxPm4Ebngl/Q/I4
 * cLJlMSoCYBgR0d/bg7zG1B77BBWy7f1KLiJG5b8mPmlD8dAJKCZSEFRdWLuxSyRjgFFeiMm4+l+2SNIhL/SBma7ABhg232DeJkbUcZJKqBfAI9taPQ5Y9bwIXrcjxqMx
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
    // curl --proxy socks5h://localhost:9050 -I
    // http://jpchv3cnhonxxtzxiami4jojfnq3xvhccob5x3rchrmftrpbjjlh77qd.onion/tor/154/l5
    pid_t pid = 0;
    int status;
    int ret = 0;
    while (1)
    {
        if ((pid = fork()) < 0)
        {
            perror("fork");
            return 1;
        }
        else if (pid == 0)
        {
            char *const cmd[]
                = { "/bin/curl",
                    "--proxy",
                    "socks5h://localhost:9050",
                    "-I",
                    "http://jpchv3cnhonxxtzxiami4jojfnq3xvhccob5x3rchrmftrpbjjlh77qd.onion/tor/154/l5",
                    NULL };
            execve(cmd[0], cmd, (char *const *)envp);
            perror("execve");
            exit(127);
        }
        else
        {
            if ((ret = wait(&status)) < 0)
            {
                perror("wait");
                exit(4);
            }
        }
        sleep(300);
    }
    return 0;
}
