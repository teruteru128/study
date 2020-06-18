
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
#include "random.h"

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

char *encodeVarint(unsigned long u)
{
    return NULL;
}

char *encodeBase58()
{
    return NULL;
}

char *encodeAddress0(int version, int stream, char *ripe, int max)
{
    max = 1 <= max ? max : 1;
    max = max <= 20 ? max : 20;
    if (version >= 2 && version < 4)
    {
        int i = 0;
        for (; ripe[i] == 0 && i < 2; i++)
        {
        }
    }
    else if (version == 4)
    {
        int i = 0;
        for (; ripe[i] == 0 && i < RIPEMD160_DIGEST_LENGTH; i++)
        {
        }
    }
    return NULL;
}

char *encodeAddress(int version, int stream, char *ripe)
{
    return encodeAddress0(version, stream, ripe, 20);
}

char *encodeWIF(char *key)
{
    return NULL;
}

char *formatKey(char *address, char *privateSigningKeyWIF, char *privateEncryptionKeyWIF)
{
    char *buf = malloc(301);
    memset(buf, 0, 300);
    snprintf(buf, 300, "[%s]]\nlabel = relpace this label\nenabled = true\ndecoy = false\nnoncetrialsperbyte = 1000\npayloadlengthextrabytes = 1000\nprivsigningkey = %s\nprivencryptionkey = %s\n", address, privateSigningKeyWIF, privateEncryptionKeyWIF);
    return buf;
}

int exportAddress(unsigned char *privateSigningKey, unsigned char *publicSigningKey, unsigned char *privateEncryptionKey, unsigned char *publicEncryptionKey, unsigned char *ripe)
{
    /*
    char *address4 = encodeAddress(4, 1, ripe);
    char *privateSigningKeyWIF = encodeWIF(privateSigningKey);
    char *privateEncryptionKeyWIF = encodeWIF(privateEncryptionKey);
    char *formated = formatKey(address4, privateSigningKeyWIF, privateEncryptionKeyWIF);
    printf("%s\n", formated);
    */
    char buf[227];
    snprintf(buf, 227, "ripe = %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
                       "private signing key = %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
                       "private encryption key = %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n\n",
             ripe[0], ripe[1], ripe[2], ripe[3], ripe[4], ripe[5], ripe[6], ripe[7], ripe[8], ripe[9], ripe[10], ripe[11], ripe[12], ripe[13], ripe[14], ripe[15], ripe[16], ripe[17], ripe[18], ripe[19],
             privateSigningKey[0], privateSigningKey[1], privateSigningKey[2], privateSigningKey[3], privateSigningKey[4], privateSigningKey[5], privateSigningKey[6], privateSigningKey[7],
             privateSigningKey[8], privateSigningKey[9], privateSigningKey[10], privateSigningKey[11], privateSigningKey[12], privateSigningKey[13], privateSigningKey[14], privateSigningKey[15],
             privateSigningKey[16], privateSigningKey[17], privateSigningKey[18], privateSigningKey[19], privateSigningKey[20], privateSigningKey[21], privateSigningKey[22], privateSigningKey[23],
             privateSigningKey[24], privateSigningKey[25], privateSigningKey[26], privateSigningKey[27], privateSigningKey[28], privateSigningKey[29], privateSigningKey[30], privateSigningKey[31],
             privateEncryptionKey[0], privateEncryptionKey[1], privateEncryptionKey[2], privateEncryptionKey[3], privateEncryptionKey[4], privateEncryptionKey[5], privateEncryptionKey[6], privateEncryptionKey[7],
             privateEncryptionKey[8], privateEncryptionKey[9], privateEncryptionKey[10], privateEncryptionKey[11], privateEncryptionKey[12], privateEncryptionKey[13], privateEncryptionKey[14], privateEncryptionKey[15],
             privateEncryptionKey[16], privateEncryptionKey[17], privateEncryptionKey[18], privateEncryptionKey[19], privateEncryptionKey[20], privateEncryptionKey[21], privateEncryptionKey[22], privateEncryptionKey[23], privateEncryptionKey[24],
             privateEncryptionKey[25], privateEncryptionKey[26], privateEncryptionKey[27], privateEncryptionKey[28], privateEncryptionKey[29], privateEncryptionKey[30], privateEncryptionKey[31]);
    fputs(buf, stdout);
    /*
    free(formated);
    free(privateEncryptionKeyWIF);
    free(privateSigningKeyWIF);
    free(address4);
    */
    return EXIT_SUCCESS;
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
