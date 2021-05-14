
#include <openssl/pem.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/pem2.h>

int main(int argc, char * argv[])
{
    if(argc < 1)
    {
        return EXIT_FAILURE;
    }

    FILE *fin = fopen(argv[1], "r");
    if(fin == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }

    RSA *rsa = PEM_read_RSA_PUBKEY(fin, NULL, NULL, NULL);
    if(rsa == NULL)
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

    const BIGNUM *e = RSA_get0_e(rsa);
    int e_len = BN_num_bytes(e);
    int e_len_be = htobe32(e_len);
    fprintf(stderr, "%08x\n", e_len);
    unsigned char *e_bin = malloc(e_len);
    BN_bn2bin(e, e_bin);

    const BIGNUM *n = RSA_get0_n(rsa);
    int n_len = BN_num_bytes(n);
    int n_len_be = htobe32(n_len + 1);
    fprintf(stderr, "%08x\n", n_len);
    unsigned char *n_bin = calloc(n_len, sizeof(unsigned char));
    BN_bn2bin(n, n_bin);

    char padding = 0;
    fwrite(&header_len_be, 4, 1, stdout);
    fwrite(header, 1, (size_t)header_len, stdout);
    fwrite(&e_len_be, 4, 1, stdout);
    fwrite(e_bin, 1, (size_t)e_len, stdout);
    fwrite(&n_len_be, 4, 1, stdout);
    fwrite(&padding, 1, 1, stdout);
    fwrite(n_bin, 1, (size_t)n_len, stdout);

    RSA_free(rsa);
    free(e_bin);
    free(n_bin);

    return EXIT_SUCCESS;
}

