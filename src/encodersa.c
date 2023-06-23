
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <fcntl.h>
#include <locale.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/params.h>
#include <openssl/types.h>
#endif

#define BUFFERSIZE 65537

void init_gettext()
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
}

int readBigNum(BIGNUM *num, const char *filename)
{
    FILE *in = fopen(filename, "r");
    if (in == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
    char buf[BUFSIZ];
    size_t buflen = 0;
    size_t length = 0;
    size_t size = BUFSIZ;
    size_t mincapa = 0;
    char *catbuf = malloc(size);
    *catbuf = 0;
    char *trash = NULL;
    char *realloctmp = NULL;
    while ((trash = fgets(buf, BUFSIZ, in)) != NULL)
    {
        buflen = strlen(buf);
        if (buflen == 0)
        {
            break;
        }
        mincapa = length + buflen + 1;
        if (mincapa > size)
        {
            while (mincapa > size)
            {
                size *= 2;
            }
            realloctmp = realloc(catbuf, size);
            if (realloctmp == NULL)
            {
                perror("realloc");
                free(catbuf);
                exit(EXIT_FAILURE);
            }
            catbuf = realloctmp;
        }
        strncat(catbuf, buf, size);
        length += buflen;
        if (strpbrk(buf, "\r\n") != NULL)
        {
            // 最初の1行だけ読み込み
            break;
        }
    }
    fclose(in);
    if (BN_hex2bn(&num, catbuf) == 0)
    {
        perror("BN_hex2bn");
        free(catbuf);
        return EXIT_FAILURE;
    }
    memset(catbuf, 0, size);
    memset(buf, 0, BUFSIZ);
    free(catbuf);
    printf("p : %dbits\n", BN_num_bits(num));
    return 0;
}

static void generate_output_filename(char *dest, size_t maxlen, int bitLength)
{
    uuid_t uuid;
    char uuidstr[UUID_STR_LEN];
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuidstr);
    snprintf(dest, maxlen, "%dbit-%s-priv.pem", bitLength, uuidstr);
}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
static EVP_PKEY *calc_RSA(EVP_PKEY *dest, BIGNUM *e, BIGNUM *p, BIGNUM *q,
                          BN_CTX *ctx)
#else
static RSA *calc_RSA(RSA *dest, BIGNUM *e, BIGNUM *p, BIGNUM *q, BN_CTX *ctx)
#endif
{
    if (BN_cmp(p, q) < 0)
    {
        printf(_("q is larger\n"));
        BN_swap(p, q);
    }
    else
    {
        printf(_("p is larger\n"));
    }
    BIGNUM *n = BN_new();
    BIGNUM *psub1 = BN_secure_new();
    BIGNUM *qsub1 = BN_secure_new();
    BIGNUM *phiN = BN_secure_new();
    BN_set_flags(phiN, BN_FLG_CONSTTIME);
    BIGNUM *d = BN_secure_new();
    BN_set_flags(d, BN_FLG_CONSTTIME);
    BIGNUM *dmp = BN_secure_new();
    BIGNUM *dmq = BN_secure_new();
    BIGNUM *iqmp = BN_secure_new();
    BIGNUM *qSecure = BN_dup(q);
    BN_set_flags(qSecure, BN_FLG_CONSTTIME | BN_FLG_SECURE);
    if (iqmp == NULL)
    {
        perror("BN_new");
        return NULL;
    }

    // 法nを計算
    if (!BN_mul(n, p, q, ctx))
        goto err2;

    printf("bit size: %d\n", BN_num_bits(n));

    // p - 1
    if (!BN_sub(psub1, p, BN_value_one()))
        goto err2;
    // q - 1
    if (!BN_sub(qsub1, q, BN_value_one()))
        goto err2;
    // オイラーのトーシェント関数
    // φ(n)計算
    if (!BN_mul(phiN, psub1, qsub1, ctx))
        goto err2;

    // 秘密指数dを計算
    if (!BN_mod_inverse(d, e, phiN, ctx))
    {
        perror(ERR_reason_error_string(ERR_get_error()));
        goto err2; /* d */
    }

    // CRTパラメータ計算
    // d mod (p-1), d mod (q-1)
    /* calculate d mod (p-1) and d mod (q - 1) */
    if (!BN_mod(dmp, d, psub1, ctx))
        goto err2;

    if (!BN_mod(dmq, d, qsub1, ctx))
        goto err2;

    // q mod p
    /* calculate inverse of q mod p */
    if (!BN_mod_inverse(iqmp, qSecure, p, ctx))
        goto err2;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PARAM_BLD *bld = OSSL_PARAM_BLD_new();

    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, n);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, e);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, d);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_FACTOR1, p);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_FACTOR2, q);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_EXPONENT1, dmp);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_EXPONENT2, dmq);
    OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_COEFFICIENT1, iqmp);

    OSSL_PARAM *param = OSSL_PARAM_BLD_to_param(bld);
    OSSL_PARAM_BLD_free(bld);

    EVP_PKEY_CTX *pkeyctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);

    EVP_PKEY_fromdata_init(pkeyctx);

    EVP_PKEY *work = NULL;
    EVP_PKEY_free(work);

    EVP_PKEY_fromdata(pkeyctx, &work, EVP_PKEY_KEYPAIR, param);

    OSSL_PARAM_free(param);
    EVP_PKEY_CTX_free(pkeyctx);
#else
    RSA *work = (dest == NULL) ? RSA_new() : dest;
    RSA_set0_key(work, n, e, d);
    RSA_set0_factors(work, p, q);
    RSA_set0_crt_params(work, dmp, dmq, iqmp);
#endif
    return work;
err2:
    BN_free(n);
    BN_free(psub1);
    BN_free(qsub1);
    BN_free(phiN);
    BN_free(d);
    BN_free(dmp);
    BN_free(dmp);
    BN_free(iqmp);
#if 0
    BN_free(n);
    BN_free(e);
    BN_free(d);
    BN_free(p);
    BN_free(q);
    BN_free(dmp);
    BN_free(dmq);
    BN_free(iqmp);
#endif
    return NULL;
}

int encode_rsa_main(const int argc, const char *argv[])
{
    init_gettext();
    int ret = EXIT_FAILURE;
    // ファイルから素数を読み込む
    if (argc < 3)
    {
        fprintf(stderr, "%s file1 file2\n", argv[0]);
        return EXIT_SUCCESS;
    }
    const char *infile1 = argv[1];
    const char *infile2 = argv[2];
    BN_CTX *ctx = BN_CTX_new();
    if (ctx == NULL)
    {
        fprintf(stderr, "BN_CTX_new : %s\n",
                ERR_reason_error_string(ERR_get_error()));
        return EXIT_FAILURE;
    }
    BN_CTX_start(ctx);
    BIGNUM *e = BN_new();
    BIGNUM *p = BN_secure_new();
    BIGNUM *q = BN_secure_new();
    if (q == NULL)
    {
        perror("BN_new");
        goto err;
    }
    if (!BN_set_word(e, RSA_F4))
    {
        perror("BN_set_word");
        goto err;
    }

    readBigNum(p, infile1);
    readBigNum(q, infile2);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_PKEY *rsa = calc_RSA(NULL, e, p, q, ctx);
#else
    RSA *rsa = calc_RSA(NULL, e, p, q, ctx);
#endif
    if (rsa == NULL)
    {
        // perror("calc_RSA");
        unsigned long r = ERR_get_error();
        fprintf(stderr, "calc_RSA : %lu %s\n", r, ERR_reason_error_string(r));
        goto err;
    }

    // ファイル書き出し

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    BIGNUM *n = NULL;
    EVP_PKEY_get_bn_param(rsa, OSSL_PKEY_PARAM_RSA_N, &n);
#else
    const BIGNUM *n = RSA_get0_n(rsa);
#endif
    int bitLength = BN_num_bits(n);
    char outfile[FILENAME_MAX];
    generate_output_filename(outfile, FILENAME_MAX, bitLength);

    FILE *fout = fopen(outfile, "w");
    if (fout == NULL)
    {
        perror("fout outfile");
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_PKEY_free(rsa);
#else
        RSA_free(rsa);
#endif
        goto err;
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    if (!PEM_write_PrivateKey(fout, rsa, NULL, NULL, 0, NULL, NULL))
#else
    if (!PEM_write_RSAPrivateKey(fout, rsa, NULL, NULL, 0, NULL, NULL))
#endif
    {
        perror(ERR_reason_error_string(ERR_get_error()));
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_PKEY_free(rsa);
#else
    RSA_free(rsa);
#endif
    fclose(fout);
    ret = EXIT_SUCCESS;
err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    if (ret != EXIT_SUCCESS)
    {
        BN_clear_free(e);
        BN_clear_free(p);
        BN_clear_free(q);
    }
    return ret;
}

/**
 * @brief
 * TODO: --enable-validate
 * @param argc
 * @param argv
 * @return int
 */
int main(const int argc, const char *argv[])
{
    return encode_rsa_main(argc, argv);
}
