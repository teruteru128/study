
#include "config.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <libintl.h>
#include <locale.h>
#define _(str) gettext(str)

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 65536ULL
#define REQUIRE_NLZ 5
#define ADDRESS_VERSION 4
#define STREAM_NUMBER 1
#define J_CACHE_SIZE 8

#define errchk(v, f)                                                \
    if (!v)                                                         \
    {                                                               \
        unsigned long err = ERR_get_error();                        \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
        return EXIT_FAILURE;                                        \
    }

int nextBytes(char *buf, size_t len)
{
    char *inf = "/dev/urandom";
    FILE *in = fopen(inf, "rb");
    if (in == NULL)
    {
        return EXIT_FAILURE;
    }

    size_t r = fread(buf, 1, len, in);

    if (len != r)
    {
        perror("fread");
        fclose(in);
        return EXIT_FAILURE;
    }
    int i = fclose(in);

    if (i != 0)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#if 0
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
    max = max <= 20 ? max : 20;
    max = max >= 1 ? max : 1;
    size_t ripelen = 20;
    if(version >= 2 && version < 4)
    {
        int i = 0;
        for(;ripe[i] == 0 && i < 2; i++){}
    } else if(version == 4)
    {
        int i = 0;
        for(;ripe[i] == 0 && i < 2; i++){}
    }
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
#endif

int exportAddress(unsigned char *privateSigningKey, unsigned char *publicSigningKey, unsigned char *privateEncryptionKey, unsigned char *publicEncryptionKey, unsigned char *ripe)
{
    /*
    char *address4 = encodeAddress(4, 1, ripe);
    char *privateSigningKeyWIF = encodeWIF(privateSigningKey);
    char *privateEncryptionKeyWIF = encodeWIF(privateEncryptionKey);
    char *formated = formatKey(address4, privateSigningKeyWIF, privateEncryptionKeyWIF);
    printf("%s\n", formated);
    */
    size_t i = 0;
    fputs("ripe = ", stdout);
    for (i = 0; i < RIPEMD160_DIGEST_LENGTH; i++)
    {
        fprintf(stdout, "%02x", ripe[i]);
    }
    fputs("\nprivate signing key = ", stdout);
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        fprintf(stdout, "%02x", privateSigningKey[i]);
    }
    fputs("\nprivate encryption key = ", stdout);
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        fprintf(stdout, "%02x", privateEncryptionKey[i]);
    }
    fputs("\n\n", stdout);
    /*
    free(formated);
    free(privateEncryptionKeyWIF);
    free(privateSigningKeyWIF);
    free(address4);
    */
    return EXIT_SUCCESS;
}

/*
    TODO: 鍵キャッシュサーバー
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
    unsigned char iPublicKey[PUBLIC_KEY_LENGTH];
    unsigned char jPublicKey[PUBLIC_KEY_LENGTH * J_CACHE_SIZE];
    unsigned char cache64[SHA512_DIGEST_LENGTH];
    size_t i = 0;
    size_t j = 0;
    size_t jj_max = 0;
    size_t jj = 0;
    int r = 0;

    // curve 生成
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    errchk(secp256k1, EC_GROUP_new_by_curve_name);
    const EC_POINT *g = EC_GROUP_get0_generator(secp256k1);
    // private key working area
    BIGNUM *prikey = BN_new();
    errchk(prikey, BN_new);
    // public key working area
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    errchk(pubkey, EC_POINT_new);
    BN_CTX *ctx = BN_CTX_new();
    errchk(ctx, BN_CTX_new);
    BIGNUM *tmp = NULL;
    SHA512_CTX sha512ctx;
    RIPEMD160_CTX ripemd160ctx;
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
            // i = jの時に同じ鍵の組み合わせを2回計算していた分を削減
            r = SHA512_Init(&sha512ctx);
            errchk(r, SHA512_Init);
            r = SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
            errchk(r, SHA512_Update);
            r = SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
            errchk(r, SHA512_Update);
            r = SHA512_Final(cache64, &sha512ctx);
            errchk(r, SHA512_Final);
            r = RIPEMD160_Init(&ripemd160ctx);
            errchk(r, RIPEMD160_Init);
            r = RIPEMD160_Update(&ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
            errchk(r, RIPEMD160_Update);
            r = RIPEMD160_Final(cache64, &ripemd160ctx);
            errchk(r, RIPEMD160_Final);
            if (!(cache64[0] || cache64[1] || cache64[2] || cache64[3] || cache64[4]))
            {
                exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
            }
            for (j = 0; j < i; j += J_CACHE_SIZE)
            {
                memcpy(jPublicKey, publicKeys + (j * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH * J_CACHE_SIZE);
                for (jj = 0; jj < J_CACHE_SIZE && (j + jj) < i; jj++)
                {
                    r = SHA512_Init(&sha512ctx);
                    errchk(r, SHA512_Init);
                    r = SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
                    errchk(r, SHA512_Update);
                    r = SHA512_Update(&sha512ctx, &jPublicKey[jj * PUBLIC_KEY_LENGTH], PUBLIC_KEY_LENGTH);
                    errchk(r, SHA512_Update);
                    r = SHA512_Final(cache64, &sha512ctx);
                    errchk(r, SHA512_Final);
                    r = RIPEMD160_Init(&ripemd160ctx);
                    errchk(r, RIPEMD160_Init);
                    r = RIPEMD160_Update(&ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
                    errchk(r, RIPEMD160_Update);
                    r = RIPEMD160_Final(cache64, &ripemd160ctx);
                    errchk(r, RIPEMD160_Final);
                    if (!(cache64[0] || cache64[1] || cache64[2] || cache64[3] || cache64[4]))
                    {
                        exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + ((j + jj) * PRIVATE_KEY_LENGTH), &jPublicKey[jj * PUBLIC_KEY_LENGTH], cache64);
                    }
                    r = SHA512_Init(&sha512ctx);
                    errchk(r, SHA512_Init);
                    r = SHA512_Update(&sha512ctx, &jPublicKey[jj * PUBLIC_KEY_LENGTH], PUBLIC_KEY_LENGTH);
                    errchk(r, SHA512_Update);
                    r = SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
                    errchk(r, SHA512_Update);
                    r = SHA512_Final(cache64, &sha512ctx);
                    errchk(r, SHA512_Final);
                    r = RIPEMD160_Init(&ripemd160ctx);
                    errchk(r, RIPEMD160_Init);
                    r = RIPEMD160_Update(&ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
                    errchk(r, RIPEMD160_Update);
                    r = RIPEMD160_Final(cache64, &ripemd160ctx);
                    errchk(r, RIPEMD160_Final);
                    if (!(cache64[0] || cache64[1] || cache64[2] || cache64[3] || cache64[4]))
                    {
                        exportAddress(privateKeys + ((j + jj) * PRIVATE_KEY_LENGTH), &jPublicKey[jj * PUBLIC_KEY_LENGTH], privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
                    }
                } // jj
            }
        }
    }
    /* DEAD CODE ***********************/
shutdown:
    free(privateKeys);
    free(publicKeys);
    BN_free(prikey);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    EVP_cleanup();
    return EXIT_SUCCESS;
}
