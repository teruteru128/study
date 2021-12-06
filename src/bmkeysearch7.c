
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <bitmessage.h>
#include <math.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int loadKey(unsigned char *memories)
{
    FILE *fin = fopen(PROJECT_SOURCE_DIR "/publicKeys.bin", "rb");
    if (fin == NULL)
    {
        perror("fopen");
        return 1;
    }
    size_t readed = fread(memories, 65, 4UL * 16777216, fin);
    if (readed < 67108864UL)
    {
        perror("fread");
        fclose(fin);
        free(memories);
        return 1;
    }
    fclose(fin);
    fin = NULL;
    return 0;
}

#define clzl(tmp) (((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3)

/**
 * @brief sanbox func.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    size_t a = 4362076160UL;
    unsigned char *memories = malloc(a);
    loadKey(memories);
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = NULL;
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned int mdlen = 0;
    unsigned long tmp = 0;

    // counts の定義で _Atomic を使うか reduction を使うかは考えどころ
    // 多分 reduction のほうが早い(要検証)
    size_t counts[21] = { 0 };
    // 67108864:4503599627370496UL:4362076160UL:( ﾟдﾟ)……？
    // 16384:268435456UL : 30秒ぐらい
    // 67108864UL
    size_t i = 0;
    size_t j = 0;
    size_t ii = 0;
    size_t ii_max = 0;
    size_t jj = 0;
    size_t jj_max = 0;
    unsigned char *sign = NULL;
#pragma omp parallel for default(none) shared(sha512, ripemd160, memories, stdout) private(hash, mdctx, mdlen, tmp, i, j, ii, ii_max, jj, jj_max, sign) reduction(+: counts[:21])
    for (i = 0; i < 4362076160UL; i += 1040)
    {
        mdctx = EVP_MD_CTX_new();
        ii_max = i + 1040;
        for (j = 0; j < 4362076160UL; j += 1040)
        {
            jj_max = j + 1040;
            for (ii = i; ii < ii_max; ii += 65)
            {
                sign = memories + ii;
                for (jj = j; jj < jj_max; jj += 65)
                {
                    EVP_DigestInit(mdctx, sha512);
                    EVP_DigestUpdate(mdctx, sign, 65);
                    EVP_DigestUpdate(mdctx, memories + jj, 65);
                    EVP_DigestFinal(mdctx, hash, &mdlen);
                    assert(mdlen == 64);
                    EVP_DigestInit(mdctx, ripemd160);
                    EVP_DigestUpdate(mdctx, hash, 64);
                    EVP_DigestFinal(mdctx, hash, &mdlen);
                    assert(mdlen == 20);
                    tmp = htobe64(*(unsigned long *)hash);
                    tmp = clzl(tmp);
                    counts[tmp]++;
                    if (tmp >= 4)
                    {
                        printf("%lu, %zu, %zu\n", tmp, ii, jj);
                        fflush(stdout);
                    }
                }
            }
        }
        EVP_MD_CTX_free(mdctx);
    }

    for (int k = 0; k < 21; k++)
    {
        printf("%d : %zu\n", k, counts[k]);
    }

    free(memories);
    memories = NULL;

    return 0;
}
