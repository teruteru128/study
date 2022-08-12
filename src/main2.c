
#define _GNU_SOURCE
#include "config.h"

#include <math.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int hmac(const char *crypto, unsigned char *key, size_t keysiz,
         unsigned char *msg, size_t msglen, unsigned char *hash, size_t *s)
{
    // TODO: EVP_MAC が OpenSSL に直接導入されるのは ver. 3.0 以降
    // OpenSSL_add_all_algorithms();
    // OpenSSL_add_all_ciphers();
    // OpenSSL_add_all_digests();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD *md = EVP_MD_fetch(NULL, "SHA-1", NULL);
#else
    const EVP_MD *md = EVP_get_digestbyname("SHA-1");
#endif
    if (md == NULL)
        return 1;
    EVP_PKEY *pkey
        = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, (int)keysiz);
    EVP_DigestSignInit(mdctx, NULL, md, NULL, pkey);
    EVP_DigestSignUpdate(mdctx, msg, msglen);
    EVP_DigestSignFinal(mdctx, hash, s);
    EVP_MD_CTX_free(mdctx);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD_free(md);
#endif
    return 0;
}

static const int DIGITS_POWER[9]
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

#define BASE32_TABLE "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"

const char base32table[33] = BASE32_TABLE;

// TODO base32decodeの実装しんどいれす^q^
// changebaseにbase32を導入？
/**
 * @brief
 * https://qiita.com/waaaaRapy/items/8549890bda4f8ad33963#%E5%8B%95%E4%BD%9C%E3%83%86%E3%82%B9%E3%83%88
 *
 * @param src
 * @param srclen
 * @param out
 * @param outlen
 * @return int
 */
int base32decode(const char *src, const size_t srclen, unsigned char **out,
                 size_t *outlen)
{
    //
    if (out == NULL || outlen == NULL)
        return 1;

    *outlen = (srclen * 5) / 8;

    unsigned char *data = malloc(srclen);
    char *j = NULL;
    for (size_t i = 0; i < srclen; i++)
    {
        if ((j = strchr(base32table, src[i])) != NULL)
        {
            data[i] = j - base32table;
        }
    }

    *out = malloc(*outlen);
    uint32_t tmp = 0;
    for (size_t i = 0, k = 0; i < srclen; i += 8, k += 5)
    {
        *out[k] = data[i + 0] << 3 | data[i + 1] >> 2;
        tmp = 0;
        for (size_t shift = 30, l = 1; shift >= 0; shift -= 5, l++)
        {
            tmp |= data[i + l] << shift;
        }
    }

    return 0;
}

int hiho(int argc, char **argv, const char **envp)
{
    printf("%p\n", printf);
    return 0;
}
