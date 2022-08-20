
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#include "yattaze.h"
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#endif

/**
 * @brief
 * ハッシュを秘密鍵とみなして署名公開鍵を生成 秘密鍵でパスフレーズに署名
 * パスフレーズと署名公開鍵を保存 saltを入れる余地は……？
 * https://www.openssl.org/docs/man3.0/man3/EVP_PKEY_fromdata.html
 * */
int main(void)
{
    const char *msg = YATTAZE;
    const EVP_MD *sha256 = EVP_sha256();
    EVP_MD_CTX *mdctx = NULL;
    int ret = 0;

    unsigned char *sig = NULL;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashlen = 0;
    size_t length1 = 0;
    size_t length2 = 0;
    size_t *slen = &length1;

    /* Create the Message Digest Context */
    if (!(mdctx = EVP_MD_CTX_new()))
        goto err;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    if (EVP_DigestInit_ex2(mdctx, sha256, NULL) != 1)
#else
    if (EVP_DigestInit_ex(mdctx, sha256, NULL) != 1)
#endif
    {
        return EXIT_FAILURE;
    }

    if (EVP_DigestUpdate(mdctx, (const unsigned char *)msg, strlen(msg)) != 1)
    {
        return 1;
    }
    if (EVP_DigestFinal(mdctx, hash, &hashlen) != 1)
    {
        return 1;
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    if (EVP_DigestInit_ex2(mdctx, sha256, NULL) != 1)
#else
    if (EVP_DigestInit_ex(mdctx, sha256, NULL) != 1)
#endif
    {
        return 1;
    }

    if (EVP_DigestUpdate(mdctx, hash, (size_t)hashlen) != 1)
    {
        return 1;
    }
    if (EVP_DigestFinal(mdctx, hash, &hashlen) != 1)
    {
        return 1;
    }

    BIGNUM *priv = BN_bin2bn(hash, (int)hashlen, NULL);

    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT *pub = EC_POINT_new(group);
    BN_CTX *ctx = BN_CTX_new();
    EC_POINT_mul(group, pub, priv, NULL, NULL, ctx);

    EVP_PKEY *pkey = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PARAM_BLD *param_bld = OSSL_PARAM_BLD_new();
    OSSL_PARAM *params = NULL;
    if (OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_PKEY_PARAM_GROUP_NAME,
                                        SN_secp256k1, 0)
        != 1)
    {
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }
    if (OSSL_PARAM_BLD_push_BN(param_bld, OSSL_PKEY_PARAM_PRIV_KEY, priv) != 1)
    {
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }
    // ec_pointからoctet stringに変換してEVP_PKEYに読み込み

    size_t buflen = EC_POINT_point2oct(
        group, pub, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, ctx);
    printf("buflen = %zu\n", buflen);
    unsigned char *buf = malloc(buflen);
    EC_POINT_point2oct(group, pub, POINT_CONVERSION_UNCOMPRESSED, buf, buflen,
                       ctx);
    OSSL_PARAM_BLD_push_octet_string(param_bld, OSSL_PKEY_PARAM_PUB_KEY, buf,
                                     buflen);

    params = OSSL_PARAM_BLD_to_param(param_bld);
    if (params == NULL)
    {
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }
    // bufのメモリ領域開放はOSSL_PARAM_BLD_to_paramより後
    memset(buf, 0, buflen);
    free(buf);
    EVP_PKEY_CTX *pkeyctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
    if (EVP_PKEY_fromdata_init(pkeyctx) != 1)
    {
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }
    if (EVP_PKEY_fromdata(pkeyctx, &pkey, EVP_PKEY_KEYPAIR, params) != 1)
    {
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }
    EVP_PKEY_CTX_free(pkeyctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(param_bld);
#else
    pkey = EVP_PKEY_new();
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_set_private_key(key, priv);
    EC_KEY_set_public_key(key, pub);
    EVP_PKEY_set1_EC_KEY(pkey, key);
#endif
    BN_CTX_free(ctx);

    /* Initialise the DigestSign operation - SHA-256 has been selected as the
     * message digest function in this example */
    if (1 != EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey))
        goto err;

    /* Call update with the message */
    if (1 != EVP_DigestSignUpdate(mdctx, msg, strlen(msg)))
        goto err;

    /* Finalise the DigestSign operation */
    /* First call EVP_DigestSignFinal with a NULL sig parameter to obtain the
     * length of the signature. Length is returned in slen */
    if (1 != EVP_DigestSignFinal(mdctx, NULL, &length1))
        goto err;
    printf("署名長さ: %zu\n", length1);
    /* Allocate memory for the signature based on size in slen */
    if (!(sig = OPENSSL_malloc(sizeof(unsigned char) * length1)))
        goto err;
    length2 = length1;
    /* Obtain the signature */
    if (1 != EVP_DigestSignFinal(mdctx, sig, &length2))
        goto err;
    printf("署名長さ: %zu\n", length2);
    printf("署名:\n");
    for (size_t i = 0; i < length2; i++)
    {
        printf("%02x", sig[i]);
        if ((i % 16) == 15)
            printf("\n");
        else if ((i % 8) == 7)
            printf(" ");
    }
    printf("\n");
    printf("length1 == length2: %d\n", length1 == length2);
    for (size_t i = length2; i < length1; i++)
    {
        printf("%02x", sig[i]);
    }
    printf("\n");
    EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestVerifyUpdate(mdctx, msg, strlen(msg));
    int verifyStatus = 0;
    if ((verifyStatus = EVP_DigestVerifyFinal(mdctx, sig, length2)) == 1)
    {
        printf("検証成功！\n");
        /* Success */
        ret = 1;
    }
    else if (verifyStatus == 0)
    {
        printf("検証ダメです！\n");
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
    }
    else
    {
        printf("検証以前に重大なエラーが発生しました！\n");
        fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
    }

    unsigned long errorcode = 0;
err:
    if (ret != 1)
    {
        /* Do some error handling */
        fprintf(stderr, "error!\n");
        while ((errorcode = ERR_get_error()) != 0)
        {
            fprintf(stderr, "%s\n", ERR_error_string(errorcode, NULL));
        }
    }
    /* Clean up */
    if (sig && !ret)
        OPENSSL_free(sig);
    if (mdctx)
        EVP_MD_CTX_free(mdctx);
    return 0;
}
