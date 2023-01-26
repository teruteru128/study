
#include <omp.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>
#include <string.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/provider.h>
#include <openssl/types.h>
#endif

#define IN                                                                    \
    "MEwDAgcAAgEgAiEA7Vo1+"                                                   \
    "Orf2xuuu6hTPAPldSfrUZZ7WYAzpRcO5DoYFLoCIF1JKVBctOGvMOy495O/"             \
    "BWFuFEYH4i1f6vU0b9+a64RD"

int main(int argc, char const *argv[])
{

    const EVP_MD *sha1 = EVP_sha1();
    EVP_MD_CTX *ctx[5];

    EVP_MD_CTX *ctx0 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx2 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx3 = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx4 = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_DigestInit_ex2(ctx0, sha1, NULL);
    EVP_DigestInit_ex2(ctx1, sha1, NULL);
    EVP_DigestInit_ex2(ctx2, sha1, NULL);
    EVP_DigestInit_ex2(ctx3, sha1, NULL);
    EVP_DigestInit_ex2(ctx4, sha1, NULL);
#else
    EVP_DigestInit_ex(ctx0, sha1, NULL);
    EVP_DigestInit_ex(ctx1, sha1, NULL);
    EVP_DigestInit_ex(ctx2, sha1, NULL);
    EVP_DigestInit_ex(ctx3, sha1, NULL);
    EVP_DigestInit_ex(ctx4, sha1, NULL);
#endif
    EVP_DigestUpdate(ctx0, IN, strlen(IN));
    char buf[5] = "";
    unsigned char hash[20];
    size_t len = 0;
    size_t j = 0;
    size_t k = 0;
    size_t l = 0;
    size_t m = 0;
    double global_start = omp_get_wtime();
    double global_finish = 0;
    double subtotal_start = 0;
    double subtotal_finish = 0;
    double start = 0;
    double finish = 0;
    for (size_t i = 16; i < 17; i++)
    {
        subtotal_start = omp_get_wtime();
        EVP_MD_CTX_copy_ex(ctx1, ctx0);
        len = snprintf(buf, 5, "%zu", i);
        EVP_DigestUpdate(ctx1, buf, len);
        for (j = 0; j < 10000; j++)
        {
            start = omp_get_wtime();
            EVP_MD_CTX_copy_ex(ctx2, ctx1);
            len = snprintf(buf, 5, "%04zu", j);
            EVP_DigestUpdate(ctx2, buf, len);
            for (k = 0; k < 10000; k++)
            {
                EVP_MD_CTX_copy_ex(ctx3, ctx2);
                len = snprintf(buf, 5, "%04zu", k);
                EVP_DigestUpdate(ctx3, buf, len);
                for (l = 0; l < 10000; l++)
                {
                    EVP_MD_CTX_copy_ex(ctx4, ctx3);
                    len = snprintf(buf, 5, "%04zu", l);
                    EVP_DigestUpdate(ctx4, buf, len);
                    EVP_DigestFinal_ex(ctx4, hash, NULL);
#if BYTE_ORDER == LITTLE_ENDIAN
                    if (*(unsigned long *)hash & 0x00001fffffffffffUL)
#elif BYTE_ORDER == BIG_ENDIAN
                    if (*(unsigned long *)hash & 0xffffffffff1f0000UL)
#else
#error "unknown endian!"
#endif
                    {
                        continue;
                    }
                    printf("%zu%04zu%04zu%04zu, %d\n", i, j, k, l,
                           __builtin_ctzl(le64toh(*(unsigned long *)hash)));
                }
            }
            finish = omp_get_wtime();
            fprintf(stderr, ">> %zu-%zu done(%lf)\n",
                    (i * 10000UL + j) * 100000000UL,
                    (i * 10000UL + j + 1) * 100000000UL, finish - start);
        }
        subtotal_finish = omp_get_wtime();
        fprintf(stderr, "> %zu-%zu done(%lf)\n", i * 1000000000000UL,
                (i + 1) * 1000000000000UL, subtotal_finish - subtotal_start);
    }
    global_finish = omp_get_wtime();
    fprintf(stderr, "done(%lf)\n", global_finish - global_start);
    EVP_MD_CTX_free(ctx0);
    EVP_MD_CTX_free(ctx1);
    EVP_MD_CTX_free(ctx2);
    EVP_MD_CTX_free(ctx3);
    EVP_MD_CTX_free(ctx4);
    return 0;
}
