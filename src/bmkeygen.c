
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
#define KEY_CACHE_SIZE 65536
#define REQUIRE_NLZ 3

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
    printf("ripe = ");
    for (i = 0; i < RIPEMD160_DIGEST_LENGTH; i++)
    {
        printf("%02x", ripe[i]);
    }
    printf("\n");
    printf("private signing key = ");
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        printf("%02x", privateSigningKey[i]);
    }
    printf("\n");
    printf("private encryption key = ");
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        printf("%02x", privateEncryptionKey[i]);
    }
    printf("\n");
    printf("\n");
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
    unsigned char *publicKeys = calloc(PUBLIC_KEY_LENGTH, KEY_CACHE_SIZE);
    if (publicKeys == NULL)
    {
        return EXIT_FAILURE;
    }
    unsigned char iPublicKey[PUBLIC_KEY_LENGTH];
    unsigned char jPublicKey[PUBLIC_KEY_LENGTH];
    unsigned char cache64[SHA512_DIGEST_LENGTH];
    nextBytes(privateKeys, KEY_CACHE_SIZE * PRIVATE_KEY_LENGTH);
    printf("%p\n", privateKeys);
    printf("%p\n", &privateKeys[PRIVATE_KEY_LENGTH]);
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
    printf("Initializing public keys...\n");
    BIGNUM *tmp = NULL;
    struct timespec start, end;
    timespec_get(&start, TIME_UTC);
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
        if (r != 1)
        {
            unsigned long err = ERR_get_error();
            fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
            return EXIT_FAILURE;
        }
        r = EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, publicKeys + (i * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH, ctx);
        if (r == 0)
        {
            unsigned long err = ERR_get_error();
            fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
            return EXIT_FAILURE;
        }
    }
    timespec_get(&end, TIME_UTC);
    printf("Public keys has been initialized.\n");
    unsigned long ss = ((unsigned long)start.tv_sec) * 1000000000UL + start.tv_nsec;
    unsigned long ee = ((unsigned long)end.tv_sec) * 1000000000UL + end.tv_nsec;
    printf("%lf keys / s\n", (1e9 * KEY_CACHE_SIZE) / (ee - ss));
    // iのキャッシュサイズは一つ
    // jのキャッシュサイズは4つ
    SHA512_CTX sha512ctx;
    RIPEMD160_CTX ripemd160ctx;
    size_t nlz;
    for (i = 0; i < KEY_CACHE_SIZE; i++)
    {
        memcpy(iPublicKey, publicKeys + (i * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
        // ヒープから直接参照するより一度スタックにコピーしたほうが早い説
        for (j = 0; j <= i; j++)
        {
            SHA512_Init(&sha512ctx);
            SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
            SHA512_Update(&sha512ctx, publicKeys + (j * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
            SHA512_Final(cache64, &sha512ctx);
            RIPEMD160_Init(&ripemd160ctx);
            RIPEMD160_Update(&ripemd160ctx, cache64, 64);
            RIPEMD160_Final(cache64, &ripemd160ctx);
            for (nlz = 0; cache64[nlz] == 0 && nlz < RIPEMD160_DIGEST_LENGTH; nlz++)
            {
            }
            if (nlz >= REQUIRE_NLZ)
            {
                exportAddress(privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, privateKeys + (j * PRIVATE_KEY_LENGTH), publicKeys + (j * PUBLIC_KEY_LENGTH), cache64);
            }
            SHA512_Init(&sha512ctx);
            SHA512_Update(&sha512ctx, publicKeys + (j * PUBLIC_KEY_LENGTH), PUBLIC_KEY_LENGTH);
            SHA512_Update(&sha512ctx, iPublicKey, PUBLIC_KEY_LENGTH);
            SHA512_Final(cache64, &sha512ctx);
            RIPEMD160_Init(&ripemd160ctx);
            RIPEMD160_Update(&ripemd160ctx, cache64, 64);
            RIPEMD160_Final(cache64, &ripemd160ctx);
            for (nlz = 0; cache64[nlz] == 0 && nlz < RIPEMD160_DIGEST_LENGTH; nlz++)
            {
            }
            if (nlz >= REQUIRE_NLZ)
            {
                exportAddress(privateKeys + (j * PRIVATE_KEY_LENGTH), publicKeys + (j * PUBLIC_KEY_LENGTH), privateKeys + (i * PRIVATE_KEY_LENGTH), iPublicKey, cache64);
            }
        }
    }
    for (i = 0; i < PRIVATE_KEY_LENGTH; i++)
    {
        printf("%02x", privateKeys[i]);
    }
    printf("\n");
    for (i = 0; i < PUBLIC_KEY_LENGTH; i++)
    {
        printf("%02x", publicKeys[i]);
    }
    printf("\n");
    /***********************************/
    free(privateKeys);
    free(publicKeys);
    BN_free(prikey);
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    EVP_cleanup();
    return EXIT_SUCCESS;
}
