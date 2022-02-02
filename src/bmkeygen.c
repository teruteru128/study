
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <locale.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define errchk(v, f)                                                          \
    if (!v)                                                                   \
    {                                                                         \
        unsigned long err = ERR_get_error();                                  \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL));           \
        return EXIT_FAILURE;                                                  \
    }

#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_SIZE 65
#define KEY_NUM 64

static char STDOUT_BUFFER[BUFSIZ * 16] = "";

int main(int argc, char *argv[], char *envp[])
{
    setlocale(LC_ALL, "");
    if (argc < 2)
    {
        return 0;
    }
    int r = 0;
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    errchk(secp256k1, EC_GROUP_new_by_curve_name);
    // private key working area
    BIGNUM *prikey = BN_new();
    errchk(prikey, BN_new);
    BN_CTX *ctx = BN_CTX_new();
    errchk(ctx, BN_CTX_new);
    size_t nlz = 0;
    unsigned char s[PRIVATE_KEY_SIZE * KEY_NUM] = "";
    BIGNUM *tmp = NULL;

    if (access(argv[1], F_OK | R_OK) != 0)
    {
        perror("file not found in access");
        BN_free(prikey);
        BN_CTX_free(ctx);
        EC_GROUP_free(secp256k1);
        return 0;
    }
    FILE *fin = fopen(argv[1], "rb");
    if (fin == NULL)
    {
        perror("fin");
        return 1;
    }

    // public key working area
    EC_POINT *pubkeyp = EC_POINT_new(secp256k1);
    errchk(pubkeyp, EC_POINT_new);
    unsigned char pubkey[PUBLIC_KEY_SIZE * KEY_NUM];
    setvbuf(stdout, STDOUT_BUFFER, _IOFBF, BUFSIZ * 16);

    size_t count = 0;
    size_t i = 0;
    while (fread(s, PRIVATE_KEY_SIZE, KEY_NUM, fin) == KEY_NUM)
    {
        for (i = 0; i < KEY_NUM; i++)
        {
            tmp = BN_bin2bn(s + PRIVATE_KEY_SIZE * i, PRIVATE_KEY_SIZE, prikey);
            errchk(tmp, BN_bin2bn);
            EC_POINT_mul(secp256k1, pubkeyp, prikey, NULL, NULL, ctx);
            EC_POINT_point2oct(secp256k1, pubkeyp,
                               POINT_CONVERSION_UNCOMPRESSED, pubkey + PUBLIC_KEY_SIZE * i, PUBLIC_KEY_SIZE, ctx);
        }
        fwrite(pubkey, PUBLIC_KEY_SIZE, KEY_NUM, stdout);
        /* 
        if ((count++ % 256) == 255)
        {
            fflush(stdout);
        }
        */
    }
    fflush(stdout);
    fclose(fin);
    EC_POINT_free(pubkeyp);
    BN_free(prikey);
    BN_CTX_free(ctx);
    /*
        EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
        EC_KEY_set_private_key(key, prikey);
        EC_KEY_set_public_key(key, pubkeyp);
        if (EC_KEY_check_key(key))
        {
            printf("ok\n");
        }
        else
        {
            printf("ng\n");
        }
        for (int i = 0; i < 65; i++)
        {
            printf("%02x", pubkey[i]);
        }
        printf("\n");
     */
    return 0;
}
