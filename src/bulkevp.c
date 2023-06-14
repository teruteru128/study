
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <openssl/evp.h>

int bulknew(EVP_MD_CTX **ctx, size_t num)
{
    for (size_t i = 0; i < num; i++)
    {
        ctx[i] = EVP_MD_CTX_new();
    }
    return 1;
}

int bulkinit_ex(EVP_MD_CTX **ctx, size_t num, const EVP_MD *type)
{
    for (size_t i = 0; i < num; i++)
    {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(ctx[i], type, NULL);
#else
        EVP_DigestInit_ex(ctx[i], type, NULL);
#endif
    }
    return 1;
}

int bulksignupdate(EVP_MD_CTX **ctx, size_t num, const void *d, size_t siz)
{
    for (size_t i = 0; i < num; i++)
    {
        EVP_DigestUpdate(ctx[i], d + siz * i, siz);
    }
    return 1;
}

int bulkencupdate(EVP_MD_CTX **ctx, size_t num, const void *d, size_t siz)
{
    for (size_t i = 0; i < num; i++)
    {
        EVP_DigestUpdate(ctx[i], d, siz);
    }
    return 1;
}

int bulkripeupdate(EVP_MD_CTX **ctx, size_t num, const void *d, size_t siz,
                   size_t stepsiz)
{
    for (size_t i = 0; i < num; i++)
    {
        EVP_DigestUpdate(ctx[i], d + stepsiz * i, siz);
    }
    return 1;
}

// mdに書き込む仕様をどうしよう…… EVP_MAX_MD_SIZE な unsgined char[]を num
// 個要求するか 1個だけ要求するか
int bulkfinal_ex(EVP_MD_CTX **ctx, size_t num, unsigned char *md)
{
    for (size_t i = 0; i < num; i++)
    {
        EVP_DigestFinal_ex(ctx[i], md + EVP_MAX_MD_SIZE * i, NULL);
    }
    return 1;
}

int bulkctxcopy(EVP_MD_CTX **out, size_t num, EVP_MD_CTX **in)
{
    for (size_t i = 0; i < num; i++)
    {
        EVP_MD_CTX_copy_ex(out[i], in[i]);
    }
    return 1;
}

int bulkfree(EVP_MD_CTX **ctx, size_t num)
{
    for (size_t i = 0; i < num; i++)
    {
        EVP_MD_CTX_free(ctx[i]);
    }
    return 1;
}

int bulkcheck(unsigned char *hash, size_t num, unsigned char *key,
              size_t yoffset, size_t x)
{
    for (size_t i = 0; i < num; i++)
    {
        if (*(uint64_t *)(hash + EVP_MAX_MD_SIZE * i)
            & 0x0000ffffffffffffUL == 0)
        {
            printf("%zu, %zu\n", yoffset + 65 * i, x);
        }
    }
}
