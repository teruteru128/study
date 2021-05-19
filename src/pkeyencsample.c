
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * 
 * @brief EVP_PKEY_encrypt sample(EVPとRSAを使って暗号化/復号)
 * @see https://www.openssl.org/docs/man1.1.1/man3/EVP_PKEY_encrypt.html
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
    EVP_PKEY_CTX *ctx;
    ENGINE *eng = NULL;
    unsigned char *out, *in;
    size_t outlen, inlen;
    EVP_PKEY *key;

    /*
     * NB: assumes eng, key, in, inlen are already set up,
     * and that key is an RSA public key
     */
    ctx = EVP_PKEY_CTX_new(key, eng);
    if (!ctx)
    {
        /* Error occurred */
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        /* Error */
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        /* Error */
    }

    /* Determine buffer length */
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen) <= 0)
    {
        /* Error */
    }

    out = OPENSSL_malloc(outlen);

    if (!out)
    {
        /* malloc failure */
    }

    if (EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen) <= 0)
    {
        /* Error */
    }

    /* Encrypted data is outlen bytes written to buffer out */
    return 0;
}
