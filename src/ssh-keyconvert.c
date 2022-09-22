
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OPENSSL_VERSION_PREREQ(3, 0)
#include <openssl/core_names.h>
#include <openssl/types.h>
#endif

int main(int argc, char *argv[])
{
    if (argc < 1)
    {
        return EXIT_FAILURE;
    }

    FILE *fin = fopen(argv[1], "r");
    if (fin == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
#if OPENSSL_VERSION_PREREQ(3, 0)
    EVP_PKEY *rsa = PEM_read_PUBKEY_ex(fin, NULL, NULL, NULL, NULL, NULL);
#else
    RSA *rsa = PEM_read_RSA_PUBKEY(fin, NULL, NULL, NULL);
#endif
    if (rsa == NULL)
    {
        perror(ERR_reason_error_string(ERR_get_error()));
        fclose(fin);
        return EXIT_FAILURE;
    }
    fclose(fin);

    const char header[] = "ssh-rsa";
    const int header_len = (int)strlen(header);
    const int header_len_be = htobe32(header_len);

    fprintf(stderr, "%08x\n", header_len);

    int e_len = 0;
    int e_len_be = 0;
    unsigned char *e_bin = NULL;
    {
#if OPENSSL_VERSION_PREREQ(3, 0)
        BIGNUM *e = NULL;
        EVP_PKEY_get_bn_param(rsa, OSSL_PKEY_PARAM_RSA_E, &e);
#else
        const BIGNUM *e = RSA_get0_e(rsa);
#endif
        e_len = BN_num_bytes(e);
        e_len_be = htobe32(e_len);
        fprintf(stderr, "%08x\n", e_len);
        e_bin = malloc(e_len);
        BN_bn2bin(e, e_bin);
#if OPENSSL_VERSION_PREREQ(3, 0)
        BN_clear_free(e);
#endif
    }

    int n_len = 0;
    int n_len_be = 0;
    unsigned char *n_bin = NULL;
    {
#if OPENSSL_VERSION_PREREQ(3, 0)
        BIGNUM *n = NULL;
        EVP_PKEY_get_bn_param(rsa, OSSL_PKEY_PARAM_RSA_N, &n);
#else
        const BIGNUM *n = RSA_get0_n(rsa);
#endif
        n_len = BN_num_bytes(n);
        n_len_be = htobe32(n_len + 1);
        fprintf(stderr, "%08x\n", n_len);
        n_bin = calloc(n_len, sizeof(unsigned char));
        BN_bn2bin(n, n_bin);
#if OPENSSL_VERSION_PREREQ(3, 0)
        BN_clear_free(n);
#endif
    }

    char padding = 0;
    fwrite(&header_len_be, 4, 1, stdout);
    fwrite(header, 1, (size_t)header_len, stdout);
    fwrite(&e_len_be, 4, 1, stdout);
    fwrite(e_bin, 1, (size_t)e_len, stdout);
    fwrite(&n_len_be, 4, 1, stdout);
    fwrite(&padding, 1, 1, stdout);
    fwrite(n_bin, 1, (size_t)n_len, stdout);

#if OPENSSL_VERSION_PREREQ(3, 0)
    EVP_PKEY_free(rsa);
#else
    RSA_free(rsa);
#endif
    free(e_bin);
    free(n_bin);

    return EXIT_SUCCESS;
}
