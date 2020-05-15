
#include <stdio.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/objects.h>

int main(int argc, char *argv[])
{
    size_t i = 0;
    const int secp256k1nid = 714;
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name (secp256k1nid);
    if(secp256k1 == NULL){
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_GROUP_new_by_curve_name : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    EC_KEY *keypair = EC_KEY_new_by_curve_name(secp256k1nid);
    if (keypair == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_KEY_new_by_curve_name : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    if (EC_KEY_generate_key(keypair) != 1)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_KEY_generate_key : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    /***********************************/
    const BIGNUM *prikey = EC_KEY_get0_private_key(keypair);
    if (prikey == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_KEY_get0_private_key : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    BN_print_fp(stdout, prikey);
    fprintf(stdout, "\n");
    /***********************************/
    EC_POINT *pubkey = EC_POINT_new(secp256k1);
    if (pubkey == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_POINT_new : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    BN_CTX* ctx = BN_CTX_new();
    if (ctx == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "BN_CTX_new : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    int r = EC_POINT_mul(secp256k1, pubkey, prikey, NULL, NULL, ctx);
    printf("EC_POINT_mul r : %d\n", r);
    if(r!=1){
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_POINT_mul : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;

    }
    r = EC_KEY_set_public_key(keypair, pubkey);
    printf("EC_KEY_set_public_key r : %d\n", r);
    if(r != 1){
        unsigned long err = ERR_get_error();
        fprintf(stderr, "EC_KEY_set_public_key : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    r = EC_POINT_is_at_infinity(secp256k1, pubkey);
    printf("EC_POINT_is_at_infinity r : %d\n", r);
    r = EC_POINT_is_on_curve(secp256k1, pubkey, ctx);
    printf("EC_POINT_is_on_curve r : %d\n", r);
    r = EC_KEY_check_key(keypair);
    printf("EC_KEY_check_key r : %d\n", r);
    int pubEncSize = i2o_ECPublicKey(keypair, NULL);
    printf("pubEncSize : %d\n", pubEncSize);
    unsigned char *encodedPublicKey = calloc(pubEncSize, 1);
    unsigned char *p = encodedPublicKey;
    printf("key : %p\n", encodedPublicKey);
    /*
    if (encodedPublicKey == NULL)
    {
        perror("calloc encodedPublicKey");
        return EXIT_FAILURE;
    }
    */
    r = i2o_ECPublicKey(keypair, &p);
    printf("i2o_ECPublicKey r : %d\n", r);
    if (r == 0)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "i2o_ECPublicKey : %s\n", ERR_error_string(err, NULL));
        return EXIT_FAILURE;
    }
    for (i = 0; i < pubEncSize; i++)
    {
        printf("%02x", encodedPublicKey[i]);
    }
    printf("\n");
    printf("key : %p\n", encodedPublicKey);
    printf("key : %p\n", p);
    /***********************************/
    BN_CTX_free(ctx);
    EC_POINT_free(pubkey);
    EC_KEY_free(keypair);
    EC_GROUP_free(secp256k1);
    /***********************************/
    /*
    size_t curve_list_size = EC_get_builtin_curves(NULL, 114514);
    if (curve_list_size == 0)
    {
        return EXIT_FAILURE;
    }
    EC_builtin_curve *list = calloc(curve_list_size, sizeof(EC_builtin_curve));
    if (list == NULL)
    {
        return EXIT_FAILURE;
    }
    size_t success_flag = EC_get_builtin_curves(list, curve_list_size);
    if (success_flag == 0)
    {
        return EXIT_FAILURE;
    }
    const char *shortname = NULL;
    const char *longname = NULL;
    for (i = 0; i < success_flag; i++)
    {
        shortname = OBJ_nid2sn(list[i].nid);
        longname = OBJ_nid2ln(list[i].nid);
        fprintf(stdout, "%d, %s, %s, %s\n", list[i].nid, longname, shortname, list[i].comment);
    }
    printf("%d\n", OBJ_sn2nid("secp256k1"));
    free(list);
    */
    return EXIT_SUCCESS;
}
