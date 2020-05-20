
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

#define PRIVATE_KEY_LENGTH 32
#define PUBLIC_KEY_LENGTH 65
#define SHA512_HASH_LENGTH 64
#define RIPEMD160_HASH_LENGTH 20
#define KEY_CACHE_SIZE 65536

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

int main(int argc, char *argv[])
{
    OpenSSL_add_all_digests();
    unsigned char *privateKeys = calloc(sizeof(char), KEY_CACHE_SIZE * PRIVATE_KEY_LENGTH);
    if (privateKeys == NULL)
    {
        return EXIT_FAILURE;
    }
    unsigned char *publicKeys = calloc(sizeof(char), KEY_CACHE_SIZE * PUBLIC_KEY_LENGTH);
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

    const int secp256k1nid = 714;
    // curve 生成
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(secp256k1nid);
    if (secp256k1 == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_GROUP_new_by_curve_name : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    // private key working area
    BIGNUM *prikey2 = BN_new();
    if (prikey2 == NULL)
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
    size_t pri_off = 0;
    size_t pub_off = 0;
    for (i = 0; i < 65536; i++, pri_off += PRIVATE_KEY_LENGTH, pub_off += PUBLIC_KEY_LENGTH)
    {
        BN_bin2bn(privateKeys + pri_off, PRIVATE_KEY_LENGTH, prikey2);

        r = EC_POINT_mul(secp256k1, pubkey, prikey2, NULL, NULL, ctx);
        if (r != 1)
        {
            unsigned long err = ERR_get_error();
            fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
            return EXIT_FAILURE;
        }
        r = EC_POINT_point2oct(secp256k1, pubkey, POINT_CONVERSION_UNCOMPRESSED, publicKeys + pub_off, PUBLIC_KEY_LENGTH, ctx);
        if (r == 0)
        {
            unsigned long err = ERR_get_error();
            fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
            return EXIT_FAILURE;
        }
    }
    printf("Public keys has been initialized.\n");
    // iのキャッシュサイズは一つ
    // jのキャッシュサイズは4つ
    for (i = 0; i < KEY_CACHE_SIZE; i++)
    {
        memcpy(iPublicKey, &publicKeys[i * PUBLIC_KEY_LENGTH], PUBLIC_KEY_LENGTH);
        // ヒープから直接参照するより一度スタックにコピーしたほうが早い説
        for (j = 0; j <= i; j++)
        {
            memcpy(jPublicKey, &publicKeys[j * PUBLIC_KEY_LENGTH], PUBLIC_KEY_LENGTH);
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
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_GROUP_free(secp256k1);
    EVP_cleanup();
    return EXIT_SUCCESS;
}
