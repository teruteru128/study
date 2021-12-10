
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

// __builtin_clzl に0を渡すと未定義になるため、そのための措置
#define clzl(tmp) ((((tmp) == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3)
#define SIZE 67108864UL
#define CTX_SIZE 4

/**
 * @brief sanbox func.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    PublicKey *memories = malloc(SIZE * PUBLIC_KEY_LENGTH);
    loadKey((unsigned char *)memories);
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *sharedmdctx[CTX_SIZE] = { NULL };
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
    PublicKey *encKey = NULL;
#pragma omp parallel for default(none) shared(sha512, ripemd160, memories, stdout) private(hash, sharedmdctx, mdctx, tmp, i, j, ii, encKey) reduction(+: counts[:21])
    for (i = 0; i < SIZE; i += CTX_SIZE)
    {
        // CTXをnewする頻度はどのくらいにしたらいいのだ？
        for (j = 0; j < CTX_SIZE; j++)
        {
            sharedmdctx[j] = EVP_MD_CTX_new();
        }
        mdctx = EVP_MD_CTX_new();
        for (j = 0; j < CTX_SIZE; j++)
        {
            EVP_DigestInit(sharedmdctx[j], sha512);
            EVP_DigestUpdate(sharedmdctx[j], memories + i + j, 65);
        }
        for (j = 0; j < SIZE; j++)
        {
            encKey = memories + j;
            for (ii = 0; ii < CTX_SIZE; ii++)
            {
                EVP_MD_CTX_copy(mdctx, sharedmdctx[ii]);
                EVP_DigestUpdate(mdctx, encKey, PUBLIC_KEY_LENGTH);
                EVP_DigestFinal(mdctx, hash, NULL);
                EVP_DigestInit(mdctx, ripemd160);
                EVP_DigestUpdate(mdctx, hash, 64);
                EVP_DigestFinal(mdctx, hash, NULL);
                tmp = htobe64(*(unsigned long *)hash);
                tmp = clzl(tmp);
                // htobe64はいるようないらないような
                // tmp = clzl(*(unsigned long *)hash);
                counts[tmp]++;
                if (tmp >= 5)
                {
                    // iiとjjから秘密鍵を読み込む
                    // 秘密鍵をWIFにエンコードする
                    // 公開鍵からアドレスにフォーマットする
                    // あとはBitMessageなりCSVなりご自由に出力する
                    printf("%lu, %zu, %zu\n", tmp, i + ii, j);
                    fflush(stdout);
                }
            }
        }

        for (j = 0; j < CTX_SIZE; j++)
        {
            EVP_MD_CTX_free(sharedmdctx[j]);
        }
        EVP_MD_CTX_free(mdctx);
    }

    for (i = 0; i < 21; i++)
    {
        printf("%zu : %zu\n", i, counts[i]);
    }

    free(memories);
    memories = NULL;

    return 0;
}
