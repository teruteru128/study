
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <stdio.h>

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

    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_set_private_key(key, priv);
    EC_KEY_set_public_key(key, pub);

    /* Initialise the DigestSign operation - SHA-256 has been selected as the
     * message digest function in this example */
    if (1 != EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, key))
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
    if (1 != EVP_DigestSignFinal(mdctx, *sig, slen))
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
