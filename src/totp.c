
#define _DEFAULT_SOURCE
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
#include <endian.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/types.h>
#endif

int hmac(const char *crypto, const unsigned char *key, const size_t keysiz,
         const unsigned char *msg, const size_t msglen, unsigned char *hash, size_t *s)
{
    // TODO: EVP_MAC が OpenSSL に直接導入されるのは ver. 3.0 以降
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", "provider=default");
    EVP_MAC_CTX *macctx = EVP_MAC_CTX_new(mac);
    OSSL_PARAM_BLD *param_bld = OSSL_PARAM_BLD_new();
    OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_MAC_PARAM_DIGEST, crypto, 0);
    OSSL_PARAM *params = OSSL_PARAM_BLD_to_param(param_bld);
    OSSL_PARAM_BLD_free(param_bld);
    EVP_MAC_init(macctx, key, keysiz, params);
    EVP_MAC_update(macctx, msg, msglen);
    EVP_MAC_final(macctx, hash, s, 20);
    EVP_MAC_CTX_free(macctx);
    OSSL_PARAM_free(params);
    EVP_MAC_free(mac);
#else
    // OpenSSL_add_all_algorithms();
    // OpenSSL_add_all_ciphers();
    // OpenSSL_add_all_digests();
    const EVP_MD *md = EVP_get_digestbyname(crypto);
    EVP_PKEY *pkey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, (int)keysiz);
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(mdctx, NULL, md, NULL, pkey);
    EVP_DigestSignUpdate(mdctx, msg, msglen);
    EVP_DigestSignFinal(mdctx, hash, s);
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
#endif
    return 0;
}

static const int32_t DIGITS_POWER[10]
    // 0  1   2    3     4      5       6        7         8          9
    = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

void generateTOTP(const unsigned char *key, const size_t keysiz, const time_t time,
                  int returnDigits, const char *crypto, char *out_str)
{
    if (returnDigits < 1 || returnDigits > 10)
        return; // 境界チェック
    uint64_t msgword = (uint64_t)time;
    unsigned char *msg = (unsigned char *)&msgword;

    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    size_t hashsiz = EVP_MAX_MD_SIZE;
    if (hmac(crypto, key, keysiz, msg, 8, hash, &hashsiz) != 0)
    {
        return;
    }

    int offset = hash[hashsiz - 1] & 0x0f;

    uint32_t val;
    memcpy(&val, hash + offset, 4);
    int32_t binary = be32toh(val) & 0x7fffffff;

    int32_t otp = binary % DIGITS_POWER[returnDigits];

    // OTP文字列生成
    snprintf(out_str, returnDigits + 1, "%0*d", returnDigits, (int)otp);
}

/**
 * @brief
 *
 * @param key base32 encoded text key
 * @param time
 * @param returnDigits
 * @return char*
 */
void generateTOTP_SHA1(const char *key, time_t time, size_t returnDigits, char *out_str)
{
    generateTOTP(key, strlen(key), time, returnDigits, "SHA-1", out_str);
}

int main(int argc, char *argv[])
{
    const time_t X = 30;
    const time_t T0 = 0;
    time_t unixtime = time(NULL);
    time_t T = (unixtime - T0) / X;
    char totp[11];
    char key[] = "3132333435363738393031323334353637383930";
    for (int i = 6; i <= 9; i++)
    {
        generateTOTP_SHA1(key, T, i, totp);
        printf("%s\n", totp);
    }
    return EXIT_SUCCESS;
}
