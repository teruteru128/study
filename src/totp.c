
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>

#include <byteswap.h>
#include <changebase.h>
#include <math.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/types.h>
#endif

int hmac(const char *crypto, unsigned char *key, size_t keysiz,
         unsigned char *msg, size_t msglen, unsigned char *hash, size_t *s)
{
    // TODO: EVP_MAC が OpenSSL に直接導入されるのは ver. 3.0 以降
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", "provider=default");
    EVP_MAC_CTX *macctx = EVP_MAC_CTX_new(mac);
    OSSL_PARAM_BLD *param_bld = OSSL_PARAM_BLD_new();
    OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_MAC_PARAM_DIGEST, crypto,
                                    0);
    OSSL_PARAM *params = OSSL_PARAM_BLD_to_param(param_bld);
    EVP_MAC_init(macctx, key, keysiz, params);
    EVP_MAC_update(macctx, msg, msglen);
    EVP_MAC_final(macctx, hash, s, 20);
    EVP_MAC_CTX_free(macctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(param_bld);
#else
    // OpenSSL_add_all_algorithms();
    // OpenSSL_add_all_ciphers();
    // OpenSSL_add_all_digests();
    const EVP_MD *md = EVP_get_digestbyname(crypto);
    EVP_PKEY *pkey
        = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, (int)keysiz);
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(mdctx, NULL, md, NULL, pkey);
    EVP_DigestSignUpdate(mdctx, msg, msglen);
    EVP_DigestSignFinal(mdctx, hash, s);
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
#endif
    return 0;
}

static const int32_t DIGITS_POWER[9]
    //  0  1   2    3     4      5       6        7         8
    = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

char *generateTOTP(unsigned char *key, size_t keysiz, time_t time,
                   int returnDigits, const char *crypto)
{
    uint64_t msgword = (uint64_t)time;
    unsigned char *msg = (unsigned char *)&msgword;

    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    size_t hashsiz = EVP_MAX_MD_SIZE;
    if (hmac(crypto, key, keysiz, msg, 8, hash, &hashsiz) != 0)
    {
        return NULL;
    }

    int offset = hash[hashsiz - 1] & 0x0f;

    int binary
        = ((hash[offset] & 0x7f) << 24) | ((hash[offset + 1] & 0xff) << 16)
          | ((hash[offset + 2] & 0xff) << 8) | (hash[offset + 3] & 0xff);

    int otp = binary % DIGITS_POWER[returnDigits];

    // テキスト整形
    char format[16] = "";
    snprintf(format, 16, "%%0%dd", returnDigits);
    char *result = malloc(returnDigits + 1);
    // 念のためゼロクリア
    memset(result, 0, returnDigits + 1);
    // OTP文字列生成
    snprintf(result, returnDigits + 1, format, otp);

    return result;
}

/**
 * @brief
 *
 * @param key base32 encoded text key
 * @param time
 * @param returnDigits
 * @return char*
 */
char *generateTOTP_SHA1(const char *key, time_t time, size_t returnDigits)
{
    return generateTOTP(key, strlen(key), time, returnDigits, "SHA-1");
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
