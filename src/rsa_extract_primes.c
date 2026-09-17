/**
 * RSA秘密鍵(PEM/DER)から素因数p, q(multi-primeならp1..pn)を取り出して標準出力へ印字する。
 *
 * 秘密鍵に触るのはこのツールだけで、計算側(hugecollatz等)へは数値だけを渡す想定。
 * 出力は既定で16進(1行1素数)。10進出力はBN_bn2dec()が2乗オーダーなので、
 * 100万bit級の素数に対しては非常に遅い点に注意。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/decoder.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ui.h>

/* multi-prime RSAでもOpenSSLが持てる素因数は10個まで(rsa-factor1 .. rsa-factor10) */
#define MAX_FACTORS 10

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [-d] [-n INDEX] <keyfile>\n"
            "  <keyfile>   RSA秘密鍵ファイル(PEM/DERは自動判別)\n"
            "  -d          10進で出力する(既定: 16進)\n"
            "  -n INDEX    INDEX番目の素数だけ出力する(1始まり)\n",
            argv0);
}

static EVP_PKEY *load_private_key(const char *path)
{
    EVP_PKEY *pkey = NULL;
    BIO *bio = BIO_new_file(path, "rb");
    if (!bio)
    {
        fprintf(stderr, "cannot open '%s': %s\n", path,
                ERR_reason_error_string(ERR_get_error()));
        return NULL;
    }

    /* 第2引数(format)をNULLにするとPEM/DERを自動判別する */
    OSSL_DECODER_CTX *dctx = OSSL_DECODER_CTX_new_for_pkey(
        &pkey, NULL, NULL, "RSA", EVP_PKEY_KEYPAIR, NULL, NULL);
    if (!dctx)
    {
        fprintf(stderr, "OSSL_DECODER_CTX_new_for_pkey failed\n");
        BIO_free(bio);
        return NULL;
    }
    /* 暗号化された鍵ならパスフレーズを対話的に尋ねる */
    OSSL_DECODER_CTX_set_passphrase_ui(dctx, UI_OpenSSL(), NULL);

    if (!OSSL_DECODER_from_bio(dctx, bio))
    {
        fprintf(stderr, "failed to decode RSA private key from '%s': %s\n", path,
                ERR_reason_error_string(ERR_get_error()));
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }

    OSSL_DECODER_CTX_free(dctx);
    BIO_free(bio);
    return pkey;
}

int main(int argc, char **argv)
{
    int decimal = 0;
    long want = 0; /* 0なら全部 */
    int i;

    for (i = 1; i < argc && argv[i][0] == '-' && argv[i][1] != '\0'; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            decimal = 1;
        }
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
        {
            want = strtol(argv[++i], NULL, 10);
            if (want < 1 || want > MAX_FACTORS)
            {
                fprintf(stderr, "-n は1〜%dの範囲で指定してください\n",
                        MAX_FACTORS);
                return EXIT_FAILURE;
            }
        }
        else
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (i != argc - 1)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    EVP_PKEY *pkey = load_private_key(argv[i]);
    if (!pkey)
    {
        return EXIT_FAILURE;
    }

    fprintf(stderr, "# %s: RSA %dbit\n", argv[i], EVP_PKEY_get_bits(pkey));

    int found = 0;
    int ret = EXIT_SUCCESS;
    for (int idx = 1; idx <= MAX_FACTORS; idx++)
    {
        char name[32];
        snprintf(name, sizeof(name), OSSL_PKEY_PARAM_RSA_FACTOR "%d", idx);

        BIGNUM *factor = NULL;
        if (!EVP_PKEY_get_bn_param(pkey, name, &factor))
        {
            ERR_clear_error();
            break; /* これ以上素因数は無い */
        }
        found++;

        if (want == 0 || want == idx)
        {
            char *str = decimal ? BN_bn2dec(factor) : BN_bn2hex(factor);
            if (!str)
            {
                fprintf(stderr, "failed to convert factor %d\n", idx);
                BN_clear_free(factor);
                ret = EXIT_FAILURE;
                break;
            }
            fprintf(stderr, "# factor%d: %dbit\n", idx, BN_num_bits(factor));
            puts(str);
            OPENSSL_free(str);
        }
        BN_clear_free(factor);
    }

    EVP_PKEY_free(pkey);

    if (found == 0)
    {
        fprintf(stderr, "no RSA factors found (公開鍵を渡していませんか?)\n");
        return EXIT_FAILURE;
    }
    if (want > found)
    {
        fprintf(stderr, "この鍵には素因数が%d個しかありません\n", found);
        return EXIT_FAILURE;
    }
    return ret;
}
