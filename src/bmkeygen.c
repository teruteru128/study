
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#define errchk(v, f)                                                \
    if (!v)                                                         \
    {                                                               \
        unsigned long err = ERR_get_error();                        \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL)); \
        return EXIT_FAILURE;                                        \
    }

int main(void)
{
    int r = 0;
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    errchk(secp256k1, EC_GROUP_new_by_curve_name);
    // private key working area
    BIGNUM *prikey = BN_new();
    errchk(prikey, BN_new);
    BN_CTX *ctx = BN_CTX_new();
    errchk(ctx, BN_CTX_new);
    BIGNUM *tmp = NULL;
    size_t nlz = 0;
    unsigned char s[32] = "";
    s[0] = 1;
    tmp = BN_bin2bn(s, 32, prikey);
    errchk(tmp, BN_bin2bn);

    // public key working area
    EC_POINT *pubkeyp = EC_POINT_new(secp256k1);
    errchk(pubkeyp, EC_POINT_new);
    EC_POINT_mul(secp256k1, pubkeyp, prikey, NULL, NULL, ctx);
    unsigned char pubkey[65];
    EC_POINT_point2oct(secp256k1, pubkeyp, POINT_CONVERSION_UNCOMPRESSED, pubkey, 65, ctx);

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
    return 0;
}
