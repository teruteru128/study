
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>

#include <byteswap.h>
#include <changebase.h>
#include <math.h>
#include <openssl/hmac.h>
#include <stdio.h>
#include <stdlib.h>

__wur char *generateTOTP(const char *key, time_t time, size_t returnDigits,
                         const EVP_MD *md)
{
    char timestr[20] = "";
    unsigned char *key1 = NULL;
    size_t keylength = parseHex(&key1, key);
    snprintf(timestr, 20, "%016lX", time);
    time = (time_t)htobe64((uint64_t)time);

    unsigned char mac[EVP_MAX_MD_SIZE + 1];
    unsigned int maclen;
    HMAC_CTX *ctx = HMAC_CTX_new();

    HMAC_Init_ex(ctx, key1, (int)keylength, md, NULL);
    HMAC_Update(ctx, (unsigned char *)&time, sizeof(time_t));
    HMAC_Final(ctx, mac, &maclen);
    HMAC_CTX_free(ctx);
    free(key1);

    size_t offset = (size_t)(mac[maclen - 1] & 0xf);

    size_t binary = (size_t)(htobe32(*(int *)(mac + offset)));

    size_t otp = binary % (size_t)pow(10, (double)returnDigits);

    char format[10];
    snprintf(format, 10, "%%0%lulu", returnDigits);
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
    char key[] = "3132333435363738393031323334353637383930";
    char *totp = generateTOTP_SHA1(key, T, 6);
    printf("%s\n", totp);
    free(totp);
    return EXIT_SUCCESS;
}
