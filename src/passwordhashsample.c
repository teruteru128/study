
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#endif

/**
 * @brief
 * ハッシュを秘密鍵とみなして署名公開鍵を生成 秘密鍵でパスフレーズに署名
 * パスフレーズと署名公開鍵を保存 saltを入れる余地は……？
 * */
int main(void)
{
    const char *msg = "yattaze.";
    EVP_MD_CTX *mdctx = NULL;
    int ret = 0;

    unsigned char *sig = NULL;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashlen = 0;
    size_t length = 0;
    size_t *slen = &length;

    /* Create the Message Digest Context */
    if (!(mdctx = EVP_MD_CTX_new()))
        goto err;

    EVP_DigestInit(mdctx, EVP_sha256());

    EVP_DigestUpdate(mdctx, (const unsigned char *)msg, strlen(msg));
    EVP_DigestFinal(mdctx, hash, &hashlen);
    EVP_DigestInit(mdctx, EVP_sha256());

    EVP_DigestUpdate(mdctx, hash, (size_t)hashlen);
    EVP_DigestFinal(mdctx, hash, &hashlen);

    BIGNUM *priv = BN_bin2bn(hash, (int)hashlen, NULL);

    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT *pub = EC_POINT_new(group);
    BN_CTX *ctx = BN_CTX_new();
    EC_POINT_mul(group, pub, priv, NULL, NULL, ctx);
    unsigned char buf[65];
    EC_POINT_point2oct(group, pub, POINT_CONVERSION_UNCOMPRESSED, buf, 65,
                       ctx);

    EVP_PKEY *pkey = EVP_PKEY_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_PKEY_set_type(pkey, EVP_PKEY_EC);
    EVP_PKEY_set_utf8_string_param(pkey, OSSL_PKEY_PARAM_GROUP_NAME,
                                   "secp256k1");
    EVP_PKEY_set_bn_param(pkey, OSSL_PKEY_PARAM_PRIV_KEY, priv);
    EVP_PKEY_set_octet_string_param(pkey, OSSL_PKEY_PARAM_PUB_KEY, buf, 65);
#else
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_set_private_key(key, priv);
    EC_KEY_set_public_key(key, pub);
    EVP_PKEY_set1_EC_KEY(pkey, key);
#endif

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
    if (1 != EVP_DigestSignFinal(mdctx, NULL, slen))
        goto err;
    /* Allocate memory for the signature based on size in slen */
    if (!(sig = OPENSSL_malloc(sizeof(unsigned char) * (*slen))))
        goto err;
    /* Obtain the signature */
    if (1 != EVP_DigestSignFinal(mdctx, sig, slen))
        goto err;

    /* Success */
    ret = 1;

err:
    if (ret != 1)
    {
        /* Do some error handling */
    }
    /* Clean up */
    if (sig && !ret)
        OPENSSL_free(sig);
    if (mdctx)
        EVP_MD_CTX_free(mdctx);
    return 0;
}
