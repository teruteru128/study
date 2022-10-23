
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

#define LIMIT 16

/**
 * @brief bitmessage鍵セット探索
 * 既存鍵を組み合わせるのってメチャクチャ面倒くさくない？
 *
 * @param argc
 * @param argv
 * @return int
 */
int searchAddressFromExistingKeys(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "%s publickeyfile1 publickeyfile2\n", argv[0]);
    }
    size_t count = 0;
    FILE *pub1 = fopen(argv[1], "rb");
    FILE *pub2 = fopen(argv[2], "rb");
    if (pub1 == NULL || pub2 == NULL)
    {
        perror("fopen");
        if (pub1)
            fclose(pub1);
        if (pub2)
            fclose(pub2);
        return 1;
    }
    struct stat filestat1;
    struct stat filestat2;
    if (fstat(fileno(pub1), &filestat1) != 0
        || fstat(fileno(pub2), &filestat2) != 0)
    {
        perror("fstat");
        fclose(pub1);
        fclose(pub2);
        return 1;
    }
    div_t d1 = div(filestat1.st_size, 65);
    div_t d2 = div(filestat2.st_size, 65);
    if (d1.rem != 0 || d2.rem != 0)
    {
        fprintf(
            stderr,
            "%sもしくは%"
            "sのどちらかのファイルサイズが公開鍵サイズの倍数ではありません\n",
            argv[1], argv[2]);
        fclose(pub1);
        fclose(pub2);
        return 1;
    }
    // 16件ずつローカル変数にコピーするのって面倒じゃない……？
    unsigned char a[1040];
    unsigned char b[1040];
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    EVP_MD_CTX *ctx0 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx2 = EVP_MD_CTX_new();
    size_t x;
    size_t y;
    size_t i;
    size_t j;
    EVP_DigestInit_ex2(ctx1, EVP_sha512(), NULL);
    unsigned char hash[EVP_MAX_MD_SIZE];
    EVP_MD *sha512 = EVP_MD_fetch(NULL, "sha512", NULL);
    EVP_MD *ripemd160 = EVP_MD_fetch(NULL, "ripemd160", NULL);
    if (ripemd160 == NULL)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "ripemd160 : %s\n", ERR_error_string(err, NULL));
        return 1;
    }
    size_t c;
    for (x = 0; x < d2.quot; x += 16)
    {
        c = fread(a, 65, 16, pub1);
        if (c != 16)
        {
            goto finish;
        }
        for (y = 0; y < d1.quot; y += 16)
        {
            c = fread(b, 65, 16, pub2);
            if (c != 16)
            {
                goto finish;
            }
            for (i = 0; i < 1040; i += 65)
            {
                EVP_DigestInit_ex2(ctx0, sha512, NULL);
                EVP_DigestUpdate(ctx0, a + i, 65);
                for (j = 0; j < 1040; j += 65)
                {
                    EVP_MD_CTX_copy(ctx1, ctx0);
                    EVP_DigestUpdate(ctx1, b + j, 65);
                    EVP_DigestFinal_ex(ctx1, hash, NULL);
                    EVP_DigestInit_ex2(ctx2, ripemd160, NULL);
                    EVP_DigestUpdate(ctx2, hash, 64);
                    EVP_DigestFinal_ex(ctx2, hash, NULL);
                    // hash[0] == 0
                    if (!hash[0])
                    {
                        printf("%zu, %zu\n", x + (i / 65), y + (j / 65));
                        count++;
                        if (count >= LIMIT)
                        {
                            goto finish;
                        }
                    }
                }
            }
        }
    }
finish:
    EVP_MD_free(sha512);
    EVP_MD_free(ripemd160);
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);

    EVP_MD_CTX_free(ctx0);
    EVP_MD_CTX_free(ctx1);
    fclose(pub1);
    fclose(pub2);
    return 0;
}
