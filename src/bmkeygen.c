
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

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define KEY_CACHE_SIZE 16777216ULL
#define REQUIRE_NLZ 5

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

#define ADDRESS_VERSION 4
#define STREAM_NUMBER 1

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

int exportAddress(unsigned char *privateSigningKey, unsigned char *publicSigningKey, unsigned char *privateEncryptionKey, unsigned char *publicEncryptionKey, unsigned char *ripe)
{
    /*
    char *address4 = encodeAddress(4, 1, ripe);
    char *privateSigningKeyWIF = encodeWIF(privateSigningKey);
    char *privateEncryptionKeyWIF = encodeWIF(privateEncryptionKey);
    char *formated = formatKey(address4, privateSigningKeyWIF, privateEncryptionKeyWIF);
    */
    //printf("%s\n", formated);
    size_t i;
    fputs("ripe = ", stdout);
    for (i = 0; i < RIPEMD160_DIGEST_LENGTH; i++)
    {
        printf("%02x", ripe[i]);
    }
    fputs("\nprivate signing key = ", stdout);
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        printf("%02x", privateSigningKey[i]);
    }
    fputs("\nprivate encryption key = ", stdout);
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        printf("%02x", privateEncryptionKey[i]);
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

int main(int argc, char *argv[])
{
    OpenSSL_add_all_digests();
    unsigned char *privateKeys = calloc(PRIVATE_KEY_LENGTH, KEY_CACHE_SIZE);
    if (privateKeys == NULL)
    {
        return EXIT_FAILURE;
    }
    unsigned char *publicKeys = calloc(sizeof(char), PUBLIC_KEY_LENGTH * KEY_CACHE_SIZE);
    if (publicKeys == NULL)
    {
        return EXIT_FAILURE;
    }
    unsigned char iPublicKey[PUBLIC_KEY_LENGTH];
    unsigned char jPublicKey[PUBLIC_KEY_LENGTH];
    unsigned char cache64[SHA512_DIGEST_LENGTH];
    size_t i = 0;
    size_t j = 0;
    int r = 0;

    // curve 生成
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (secp256k1 == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_GROUP_new_by_curve_name : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    const EC_POINT *g = EC_GROUP_get0_generator(secp256k1);
    // private key working area
    BIGNUM *prikey = BN_new();
    if (prikey == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "BN_new : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    // public key working area
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    if (pubkey == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_POINT_new : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    BN_CTX *ctx = BN_CTX_new();
    if (ctx == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "BN_CTX_new : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    BIGNUM *tmp = NULL;
    SHA512_CTX sha512ctx;
    RIPEMD160_CTX ripemd160ctx;
    size_t nlz = 0;
    while(true)
    {
        nextBytes(privateKeys, KEY_CACHE_SIZE * PRIVATE_KEY_LENGTH);
        for (i = 0; i < KEY_CACHE_SIZE; i++)
        {
            tmp = BN_bin2bn(privateKeys + (i * PRIVATE_KEY_LENGTH), PRIVATE_KEY_LENGTH, prikey);
            if (!tmp)
            {
                unsigned long err = ERR_get_error();
                fprintf(stderr, "BN_bin2bn : %s\n", ERR_error_string(err, NULL));
                return EXIT_FAILURE;
            }

            r = EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, ctx);
            if (!r)
            {
                unsigned long err = ERR_get_error();
                fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
                return EXIT_FAILURE;
            }
            r = EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, publicKeys + (i * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH, ctx);
            if (!r)
            {
                unsigned long err = ERR_get_error();
                fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
                return EXIT_FAILURE;
            }
        }
        // iのキャッシュサイズは一つ
        // jのキャッシュサイズは4つ
        for (i = 0; i < KEY_CACHE_SIZE; i++)
        {
            // ヒープから直接参照するより一度スタックにコピーしたほうが早い説
            memcpy(iPublicKey, publicKeys + (i * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
            for (j = 0; j <= i; j++)
            {
                memcpy(jPublicKey, publicKeys + (j * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
                r = SHA512_Init(&sha512ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Init : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Update : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = SHA512_Update(&sha512ctx, jPublicKey, PUBLIC_KEY_LENGTH);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Update : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = SHA512_Final(cache64, &sha512ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Final : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = RIPEMD160_Init(&ripemd160ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "RIPEMD160_Init : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = RIPEMD160_Update(&ripemd160ctx, cache64, 64);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "RIPEMD160_Update : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = RIPEMD160_Final(cache64, &ripemd160ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "RIPEMD160_Final : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                for (nlz = 0; cache64[nlz] == 0 && nlz < RIPEMD160_DIGEST_LENGTH; nlz++){}
                if (nlz >= REQUIRE_NLZ)
                {
                    exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + (j * PRIVATE_KEY_LENGTH), jPublicKey, cache64);
                }
                r = SHA512_Init(&sha512ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Init : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = SHA512_Update(&sha512ctx, jPublicKey, PUBLIC_KEY_LENGTH);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Update : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Update : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = SHA512_Final(cache64, &sha512ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "SHA512_Final : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = RIPEMD160_Init(&ripemd160ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "RIPEMD160_Init : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = RIPEMD160_Update(&ripemd160ctx, cache64, 64);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "RIPEMD160_Update : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                r = RIPEMD160_Final(cache64, &ripemd160ctx);
                if(!r){unsigned long err = ERR_get_error();fprintf(stderr, "RIPEMD160_Final : %s\n", ERR_error_string(err, NULL));return EXIT_FAILURE;}
                for (nlz = 0; cache64[nlz] == 0 && nlz < RIPEMD160_DIGEST_LENGTH; nlz++)
                {
                }
                if (nlz >= REQUIRE_NLZ)
                {
                    exportAddress(privateKeys + (j * PRIVATE_KEY_LENGTH), jPublicKey, privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
                }
            }
        }
    }
    /* DEAD CODE ***********************/
    free(privateKeys);
    free(publicKeys);
    BN_free(prikey);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    EVP_cleanup();
    return EXIT_SUCCESS;
}
