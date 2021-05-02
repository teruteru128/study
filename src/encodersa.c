
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <openssl/bn.h>
#define _(str) gettext(str)
#include <fcntl.h>
#include <locale.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>

#define CONST_E 65537
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
    size_t catlen = 0;
    size_t catcapacity = BUFSIZ;
    size_t mincapa = 0;
    char *catbuf = malloc(BUFSIZ);
    char *trash = NULL;
    char *realloctmp = NULL;
    while ((trash = fgets(buf, BUFSIZ, in)) != NULL)
    {
        buflen = strlen(buf);
        if (buflen == 0)
        {
            break;
        }
        mincapa = catcapacity + buflen + 1;
        if (mincapa > catcapacity)
        {
            while (mincapa > catcapacity)
            {
                catcapacity *= 2;
            }
            realloctmp = realloc(catbuf, catcapacity);
            if (realloctmp == NULL)
            {
                perror("realloc");
                free(catbuf);
                exit(EXIT_FAILURE);
            }
            catbuf = realloctmp;
        }
        strncat(catbuf + catlen, buf, catcapacity - (catlen + 1));
        catlen += buflen;
        if (strpbrk(buf, "\r\n") != NULL)
        {
            // 最初の1行だけ読み込み
            break;
        }
    }
    fclose(in);
    if (BN_hex2bn(&num, buf) == 0)
    {
        perror("a");
        free(catbuf);
        return EXIT_FAILURE;
    }
    memset(catbuf, 0, catcapacity);
    free(catbuf);
    printf("p : %dbits\n", BN_num_bits(num));
    return 0;
}

int calc_phi_n(BIGNUM *phiN, BIGNUM *p, BIGNUM *q, BN_CTX *ctx)
{
    BIGNUM *psub1 = BN_secure_new();
    BIGNUM *qsub1 = BN_secure_new();
    const BIGNUM *constOne = BN_value_one();

    // p - 1
    BN_sub(psub1, p, constOne);
    // q - 1
    BN_sub(qsub1, q, constOne);

    // (p - 1)(q - 1)
    BN_mul(phiN, psub1, qsub1, ctx);

    BN_free(psub1);
    BN_free(qsub1);

    return EXIT_SUCCESS;
}

static void generate_output_filename(char *dest, size_t maxlen, int bitLength)
{
    uuid_t uuid;
    char uuidstr[UUID_STR_LEN];
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuidstr);
    snprintf(dest, maxlen, "%dbit-%s-priv.pem", bitLength, uuidstr);
}

int encode_rsa_main(const int argc, const char *argv[])
{
    init_gettext();
    int ret = EXIT_FAILURE;
    // ファイルから素数を読み込む
    if (argc != 3)
    {
        printf("%s file1 file2\n", argv[0]);
        return EXIT_SUCCESS;
    }
    const char *infile1 = argv[1];
    const char *infile2 = argv[2];
    BN_CTX *ctx = BN_CTX_new();
    if (ctx == NULL)
    {
        perror("BN_CTX_new");
        goto err;
    }
    BN_CTX_start(ctx);
    BIGNUM *phiN = NULL;
    BIGNUM *psub1 = NULL;
    BIGNUM *qsub1 = NULL;
    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();
    BIGNUM *d = BN_secure_new();
    BIGNUM *p = BN_secure_new();
    BIGNUM *q = BN_secure_new();
    BIGNUM *dmp = BN_secure_new();
    BIGNUM *dmq = BN_secure_new();
    BIGNUM *iqmp = BN_secure_new();
    if (iqmp == NULL)
    {
        perror("BN_new");
        goto err;
    }
    if (!BN_set_word(e, CONST_E))
    {
        perror("BN_set_word");
        goto err;
    }
    phiN = BN_CTX_get(ctx);
    psub1 = BN_CTX_get(ctx);
    qsub1 = BN_CTX_get(ctx);
    if (qsub1 == NULL)
    {
        perror("BN_CTX_get");
    }

    readBigNum(p, infile1);
    readBigNum(q, infile2);

    // 並び替え
    if (BN_cmp(p, q) < 0)
    {
        printf(_("q is larger\n"));
        BN_swap(p, q);
    }
    else
    {
        printf(_("p is larger\n"));
    }

    // 法nを計算
    BN_mul(n, p, q, ctx);

    // p - 1
    if (!BN_sub(psub1, p, BN_value_one()))
        goto err;
    // q - 1
    if (!BN_sub(qsub1, q, BN_value_one()))
        goto err;
    // オイラーのトーシェント関数
    // φ(n)計算
    calc_phi_n(phiN, p, q, ctx);

    // 秘密指数dを計算
    BN_set_flags(phiN, BN_FLG_CONSTTIME | BN_FLG_SECURE);
    if (!BN_mod_inverse(d, e, phiN, ctx))
    {
        perror(ERR_reason_error_string(ERR_get_error()));
        goto err; /* d */
    }

    // CRTパラメータ計算
    // d mod (p-1), d mod (q-1)
    BN_set_flags(d, BN_FLG_CONSTTIME | BN_FLG_SECURE);

    /* calculate d mod (p-1) and d mod (q - 1) */
    if (!BN_mod(dmp, d, psub1, ctx))
    {
        BN_free(d);
        goto err;
    }

    if (!BN_mod(dmq, d, qsub1, ctx))
    {
        BN_free(d);
        goto err;
    }

    // q mod p
    BN_set_flags(p, BN_FLG_CONSTTIME | BN_FLG_SECURE);

    /* calculate inverse of q mod p */
    if (!BN_mod_inverse(iqmp, q, p, ctx))
    {
        goto err;
    }

    printf("bit size: %d\n", BN_num_bits(n));
    RSA *rsa = RSA_new();
    if (rsa == NULL)
    {
        perror("RSA_new");
        goto err;
    }
    RSA_set0_key(rsa, n, e, d);
    RSA_set0_factors(rsa, p, q);
    RSA_set0_crt_params(rsa, dmp, dmq, iqmp);
#if 0
    // 検証
    int err = RSA_check_key(rsa);
    printf("RSA_check_key is %d\n", err);
    if (err == 1)
    {
        printf("RSA is OK\n");
    }
    else if (err == 0)
    {
        printf("RSA is NG : %s\n", ERR_reason_error_string(ERR_get_error()));
        goto err;
    }
    else
    {
        perror(ERR_reason_error_string(ERR_get_error()));
        goto err;
    }
#endif
    // ファイル書き出し

    int bitLength = BN_num_bits(n);
    char outfile[FILENAME_MAX];
    generate_output_filename(outfile, FILENAME_MAX, bitLength);

    FILE *fout = fopen(outfile, "w");
    if(fout == NULL)
    {
        perror("fout outfile");
        goto err;
    }
    BIO *bio = BIO_new_file(outfile, "w");
    if (bio == NULL)
    {
        perror(ERR_reason_error_string(ERR_get_error()));
        goto err;
    }
    if (!PEM_write_RSAPrivateKey(fout, rsa, NULL, NULL, 0, NULL, NULL))
    {
        perror(ERR_reason_error_string(ERR_get_error()));
    }
    if (!PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL))
    {
        perror(ERR_reason_error_string(ERR_get_error()));
    }
    BIO_flush(bio);
    BIO_free(bio);
    ret = EXIT_SUCCESS;
    RSA_free(rsa);
err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
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
