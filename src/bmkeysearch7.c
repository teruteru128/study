
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <bitmessage.h>
#include <bm.h>
#include <math.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int loadPublicKey(unsigned char *memories, char *publicKeyFilePath)
{
    FILE *fin = fopen(publicKeyFilePath, "rb");
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

// __builtin_clzl に0を渡すと未定義になるため、そのための措置
#define clzl(tmp) ((((tmp) == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3)
#define SIZE 67108864UL

/**
 * @brief sanbox func.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s [public key file]", argv[0]);
        return 1;
    }
    PublicKey *memories = malloc(65UL * 4 * 16777216);
    loadPublicKey((unsigned char *)memories, argv[1]);
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = NULL;
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned long tmp = 0;

    // counts の定義で _Atomic を使うか reduction を使うかは考えどころ
    // 多分 reduction のほうが早い(要検証)
    size_t counts[21] = { 0 };
    // 67108864:4503599627370496UL:4362076160UL:( ﾟдﾟ)……？
    // 16384:268435456UL:1064960UL : 30秒ぐらい
    // 67108864UL
    size_t i = 0;
    size_t j = 0;
    size_t ii = 0;
    size_t ii_max = 0;
    size_t jj = 0;
    size_t jj_max = 0;
    PublicKey *sign = NULL;
    char *address = NULL;
#pragma omp parallel for default(none) shared(sha512, ripemd160, memories, stdout) private(hash, mdctx, tmp, i, j, ii, ii_max, jj, jj_max, sign, address) reduction(+: counts[:21])
    for (i = 0; i < SIZE; i += 16)
    {
        // CTXをnewする頻度はどのくらいにしたらいいのだ？
        mdctx = EVP_MD_CTX_new();
        ii_max = i + 16;
        for (j = 0; j < SIZE; j += 16)
        {
            jj_max = j + 16;
            for (ii = i; ii < ii_max; ii++)
            {
                sign = memories + ii;
                for (jj = j; jj < jj_max; jj++)
                {
                    EVP_DigestInit(mdctx, sha512);
                    EVP_DigestUpdate(mdctx, sign, 65);
                    EVP_DigestUpdate(mdctx, memories + jj, 65);
                    EVP_DigestFinal(mdctx, hash, NULL);
                    EVP_DigestInit(mdctx, ripemd160);
                    EVP_DigestUpdate(mdctx, hash, 64);
                    EVP_DigestFinal(mdctx, hash, NULL);
                    tmp = htobe64(*(unsigned long *)hash);
                    tmp = clzl(tmp);
                    // htobe64はいるようないらないような
                    // tmp = clzl(*(unsigned long *)hash);
                    counts[tmp]++;
                    if (tmp >= 1)
                    {
                        // iiとjjから秘密鍵を読み込む
                        // 秘密鍵をWIFにエンコードする
                        // 公開鍵からアドレスにフォーマットする
                        // あとはBitMessageなりCSVなりご自由に出力する
                        address = encodeV4Address(hash, 20);
                        if (strcasestr(address, "ninja") != 0)
                        {
                            printf("%lu, %zu, %zu, %s\n", tmp, ii, jj,
                                   address);
                            fflush(stdout);
                        }
                        free(address);
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
