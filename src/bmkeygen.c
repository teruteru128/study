
#include "study-config.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <random.h>
#include <bm.h>

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 16777216ULL
#define REQUIRE_NLZ 5
#define ADDRESS_VERSION 4
#define STREAM_NUMBER 1
#define J_CACHE_SIZE 126

#define errchk(v, f)                                                \
    if (!v)                                                         \
    {                                                               \
        unsigned long err = ERR_get_error();                        \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
        return EXIT_FAILURE;                                        \
    }

/*
 * TODO: リファクタリング
 * TODO: 鍵キャッシュサーバー
 * TODO: 既存鍵を使ってアドレス探索
 */
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    OpenSSL_add_all_digests();
    unsigned char *privateKeys = calloc(KEY_CACHE_SIZE, PRIVATE_KEY_LENGTH);
    if (!privateKeys)
    {
        perror("calloc(privateKeys)");
        return EXIT_FAILURE;
    }
    unsigned char *publicKeys = calloc(KEY_CACHE_SIZE, PUBLIC_KEY_LENGTH);
    if (!publicKeys)
    {
        perror("calloc(publicKeys)");
        return EXIT_FAILURE;
    }
    EVP_MD_CTX *sha512ctx = EVP_MD_CTX_new();
    EVP_MD_CTX *ripemd160ctx = EVP_MD_CTX_new();
    const EVP_MD *sha512md = EVP_get_digestbynid(NID_sha512);
    const EVP_MD *ripemd160md = EVP_get_digestbynid(NID_ripemd160);
    unsigned char iPublicKey[PUBLIC_KEY_LENGTH];
    unsigned char jPublicKey[PUBLIC_KEY_LENGTH * J_CACHE_SIZE];
    unsigned char cache64[EVP_MAX_MD_SIZE];
    size_t i = 0;
    size_t j = 0;
    size_t jj = 0;
    int r = 0;

    // curve 生成
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    errchk(secp256k1, EC_GROUP_new_by_curve_name);
    // private key working area
    BIGNUM *prikey = BN_new();
    errchk(prikey, BN_new);
    // public key working area
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    errchk(pubkey, EC_POINT_new);
    BN_CTX *ctx = BN_CTX_new();
    errchk(ctx, BN_CTX_new);
    BIGNUM *tmp = NULL;
    size_t nlz = 0;
    while (true)
    {
        fprintf(stderr, _("Initializing private key...\n"));
        nextBytes(privateKeys, KEY_CACHE_SIZE * PRIVATE_KEY_LENGTH);
        fprintf(stderr, _("Initialized the private key. Initialize the public key.\n"));
        // 公開鍵の生成に非常に時間がかかるので注意。秒速9000鍵で30分程度
        for (i = 0; i < KEY_CACHE_SIZE; i++)
        {
            tmp = BN_bin2bn(privateKeys + (i * PRIVATE_KEY_LENGTH), PRIVATE_KEY_LENGTH, prikey);
            errchk(tmp, BN_bin2bn);
            r = EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, ctx);
            errchk(r, EC_POINT_mul);
            r = EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, publicKeys + (i * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH, ctx);
            errchk(r, EC_POINT_point2oct);
        }
        fprintf(stderr, _("The public key initialization is complete.\n"));
        // iのキャッシュサイズは一つ
        // jのキャッシュサイズは4つ
        for (i = 0; i < KEY_CACHE_SIZE; i++)
        {
            // ヒープから直接参照するより一度スタックにコピーしたほうが早い説
            memcpy(iPublicKey, publicKeys + (i * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
            EVP_DigestInit(sha512ctx, sha512md);
            EVP_DigestUpdate(sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
            EVP_DigestUpdate(sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
            EVP_DigestFinal(sha512ctx, cache64, NULL);
            EVP_DigestInit(ripemd160ctx, ripemd160md);
            EVP_DigestUpdate(ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
            EVP_DigestFinal(ripemd160ctx, cache64, NULL);
            for (nlz = 0; !cache64[nlz] && nlz < RIPEMD160_DIGEST_LENGTH; nlz++)
            {
            }
            if (nlz >= REQUIRE_NLZ)
            {
                exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
            }
            for (j = 0; j < i; j += J_CACHE_SIZE)
            {
                memcpy(jPublicKey, publicKeys + (j * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH * J_CACHE_SIZE);
                for (jj = 0; jj < J_CACHE_SIZE && (j + jj) < i; jj++)
                {
                    EVP_DigestInit(sha512ctx, sha512md);
                    EVP_DigestUpdate(sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
                    EVP_DigestUpdate(sha512ctx, jPublicKey + (jj * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
                    EVP_DigestFinal(sha512ctx, cache64, NULL);
                    EVP_DigestInit(ripemd160ctx, ripemd160md);
                    EVP_DigestUpdate(ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
                    EVP_DigestFinal(ripemd160ctx, cache64, NULL);
                    for (nlz = 0; !cache64[nlz] && nlz < RIPEMD160_DIGEST_LENGTH; nlz++)
                    {
                    }
                    if (nlz >= REQUIRE_NLZ)
                    {
                        exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + ((j + jj) * PRIVATE_KEY_LENGTH), jPublicKey + (jj * PUBLIC_KEY_LENGTH), cache64);
                    }
                    EVP_DigestInit(sha512ctx, sha512md);
                    EVP_DigestUpdate(sha512ctx, jPublicKey + (jj * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
                    EVP_DigestUpdate(sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
                    EVP_DigestFinal(sha512ctx, cache64, NULL);
                    EVP_DigestInit(ripemd160ctx, ripemd160md);
                    EVP_DigestUpdate(ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
                    EVP_DigestFinal(ripemd160ctx, cache64, NULL);
                    for (nlz = 0; !cache64[nlz] && nlz < RIPEMD160_DIGEST_LENGTH; nlz++)
                    {
                    }
                    if (nlz >= REQUIRE_NLZ)
                    {
                        exportAddress(privateKeys + ((j + jj) * PRIVATE_KEY_LENGTH), jPublicKey + (jj * PUBLIC_KEY_LENGTH), privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
                    }
                } // jj
            }
        }
    }
    /* DEAD CODE ***********************/
//shutdown:
    free(privateKeys);
    free(publicKeys);
    BN_free(prikey);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    EVP_MD_CTX_free(ripemd160ctx);
    EVP_MD_CTX_free(sha512ctx);
    EVP_cleanup();
    return EXIT_SUCCESS;
}
