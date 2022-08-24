
#define _GNU_SOURCE

#include <openssl/err.h>
#include <openssl/evp.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/types.h>
#endif

/*
 * 秘密鍵かな？
 * ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc=
 * cm2E2vmE0Nd8aVP/4Ph2S1R6C5bkC1H7CiUBzbcDG3U=
 * BixgbLYk35GP+XHYdK/DgSWIUXyCTwCwEtY4h/G22dw=
 * BH4RDmdo0Yq0Ftiw0lm9ej5BmpZ35kEw2kaWZlZ0Do8=
 * lMhxDh6RPpWOsnJMeS12pTJ/j7EPn+ugpdbNQCGbiwc=
 * 9hZn+KDlwjgrbyFpaX5ibuaO4QfaFbIL79NUrwJlcRQ=
 * T+tDF4I700WFkFhGieYxWgQKPO/MDcntDYzMrqQSZjzwV2DzaI1OM/CsJWE30WBqMI1SxbEQHufR1A76I7ayWN==
 * nySkaCQxGErccmiqLznSQduXgFICpjnl2bo7n3FAhQMlku79plIeL85/etpN865GAnlUpErSppEYHvn4couGh3==
 * ns2bQQ4zlnfcCTSAxEH3gDDYHcBswKw92jQeEgm+9tse74XdX+LNwgfw7OsMUjOGtLMb7R/kXNRXYv1AHi71iV==
 * NxhJ5JwWhUtUccCfJNtVqzdpCMGOaAtknmcEKLyglZFNXE66EiFi9wPFekwekx3ln8m9v5wnfv7V8jSrpZ/SHQ==
 * +3n5qDbtpicXBy+Yyol/TJkg2IoQ01vZ/U2SvgpP+Fdm4DrIYngY7X0ZS53rc/KKIHT//jVqNwNBz1sGFyYUDg==
 * cLtHGFI7X/Xl6Ly03DczMzl2bsHJmI2BMQKKCckUek5vTIiltDPfT3PxdT6zxW1LzwVqJIsQEkxxPNTswgpSFg==
 * pMQBNF+F12AXT3T0mQq7S0l1VcCr/Dw2Q54zeuHH0/1ExLgbhHEsmAHf3WR9nK/Ku1Mc/eU3vaAO78yplJB76A==
 * QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQQ==
 */
int hiho(int argc, char **argv, const char **envp)
{
    EVP_PKEY_CTX *fromctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
    OSSL_PARAM_BLD *param_bld = OSSL_PARAM_BLD_new();
    OSSL_PARAM *params = NULL;
    // :thinking:
    unsigned char pub_data[]
        = { 0x00, 0x88, 0xf7, 0x5b, 0x93, 0x99, 0xfe, 0x90, 0xff, 0xf0, 0x69,
            0x2e, 0x63, 0x5b, 0xf6, 0xc9, 0xcb, 0x35, 0x74, 0x70, 0x3b, 0x5f,
            0x16, 0xab, 0xcd, 0x39, 0xd0, 0xad, 0xba, 0xc6, 0x5c, 0x01, 0xbb,
            0x98, 0x87, 0x24, 0x93, 0xbb, 0x37, 0xf2, 0x44, 0x31, 0x23, 0xbd,
            0x65, 0x76, 0x61, 0x79, 0x56, 0x8e, 0x8b, 0xc3, 0xe7, 0xe7, 0x28,
            0xf1, 0x66, 0x90, 0xb9, 0x1f, 0x05, 0x26, 0x8c, 0xb1, 0x8b };

    OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_PKEY_PARAM_GROUP_NAME,
                                    "secp256k1", 0);
    OSSL_PARAM_BLD_push_octet_string(param_bld, OSSL_PKEY_PARAM_PUB_KEY,
                                     pub_data, sizeof(pub_data));
    params = OSSL_PARAM_BLD_to_param(param_bld);
    if (params == NULL)
    {
        printf("params is null\n");
        return 1;
    }
    EVP_PKEY_fromdata_init(fromctx);
    EVP_PKEY *pkey = NULL;
    if (EVP_PKEY_fromdata(fromctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) != 1)
    {
        fprintf(stderr, "err EVP_PKEY_fromdata %s\n", ERR_reason_error_string(ERR_get_error()));
        return 1;
    }
    EVP_PKEY_CTX_free(fromctx);

    EVP_PKEY_CTX *checkctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);
    if (checkctx == NULL)
    {
        printf("err EVP_PKEY_CTX_new_from_pkey\n");
        return 1;
    }
    int ret = EVP_PKEY_public_check(checkctx);
    printf("%d\n", ret);
    EVP_PKEY_CTX_free(checkctx);

    return 0;
}
