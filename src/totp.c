
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/hmac.h>
#include <changebase.h>
#include <math.h>
#include <byteswap.h>

char *generateTOTP(const char *key, time_t time, size_t returnDigits, const EVP_MD *md)
{
    char timestr[17] = "";
    unsigned char *key1 = NULL;
    size_t keylength = parseHex(&key1, key);
    snprintf(timestr, 17, "%016lX", time);
    time = (time_t)htobe64((uint64_t)time);

    unsigned char mac[EVP_MAX_MD_SIZE + 1];
    unsigned int maclen;
    HMAC(md, key1, (int)keylength, (void *)&time, 8, mac, &maclen);
    free(key1);

    unsigned char offset = mac[maclen - 1] & 0xf;

    size_t binary = (size_t)(((mac[offset] & 0x7f) << 24) |
                             ((mac[offset + 1] & 0xff) << 16) |
                             ((mac[offset + 2] & 0xff) << 8) |
                             ((mac[offset + 3] & 0xff) << 0));

    size_t otp = binary % (size_t)pow(10, (double)returnDigits);

    char format[10];
    snprintf(format, 9, "%%0%lulu", returnDigits);
    char *ret = calloc(returnDigits + 1, sizeof(char));
    // "%0xd"
    snprintf(ret, returnDigits + 1, format, otp);

    return ret;
}

char *generateTOTP_SHA1(const char *key, time_t time, size_t returnDigits)
{
    return generateTOTP(key, time, returnDigits, EVP_sha1());
}

int main(int argc, char *argv[])
{
    const time_t X = 30;
    const time_t T0 = 0;
    time_t unixtime = time(NULL);
    time_t T = (unixtime - T0) / X;
    char *totp = generateTOTP_SHA1("3132333435363738393031323334353637383930", T, 6);
    printf("%s\n", totp);
    free(totp);
    return EXIT_SUCCESS;
}
