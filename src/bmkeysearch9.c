
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

int loadPublicKey(unsigned char *memories)
{
    FILE *fin = fopen(PROJECT_SOURCE_DIR "/publicKeys.bin", "rb");
    if (fin == NULL)
    {
        perror("fopen1");
        return 1;
    }
    size_t readed = fread(memories, PUBLIC_KEY_LENGTH, 4UL * 16777216, fin);
    if (readed < 67108864UL)
    {
        perror("fread1");
        fclose(fin);
        free(memories);
        return 1;
    }
    fclose(fin);
    fin = NULL;
    return 0;
}

int loadPrivateKey(unsigned char *privateKey)
{
    char filepath[PATH_MAX] = "";
    FILE *fin = NULL;
    size_t readed = 0;
    for (int i = 0; i < 4; i++)
    {
        fin = fopen(filepath, "rb");
        if (fin == NULL)
        {
            perror("fopen2");
            return 1;
        }
        readed = fread(privateKey + 16777216 * PRIVATE_KEY_LENGTH * i,
                       PRIVATE_KEY_LENGTH, 16777216, fin);
        if (readed < 16777216UL)
        {
            perror("fread2");
            fclose(fin);
            free(privateKey);
            return 1;
        }
        fclose(fin);
    }
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
    if (memories == NULL)
    {
        perror("malloc");
        return 1;
    }
    PrivateKey *privateKey = malloc(SIZE * PRIVATE_KEY_LENGTH);
    if (privateKey == NULL)
    {
        perror("malloc");
        return 1;
    }
    loadPublicKey((unsigned char *)memories);
    if (loadPrivateKey((unsigned char *)privateKey))
    {
        free(memories);
        memset(privateKey, 0, PRIVATE_KEY_LENGTH * SIZE);
        free(privateKey);
        return 1;
    }
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    // 将来的に512bitより大きなハッシュ関数も出てくるんかね……まあ出てくるんやろな……
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
    PublicKey *signKey = NULL;
    PublicKey *encKey = NULL;
    char *signWIF = NULL;
    char *encWIF = NULL;
    char *address = NULL;
    char *keu = NULL;
    for (i = 0; i < 1; i++)
    {
        signKey = memories + i;
        for (j = 0; j < SIZE; j++)
        {
            EVP_DigestInit(mdctx, sha512);
            EVP_DigestUpdate(mdctx, signKey, PUBLIC_KEY_LENGTH);
            EVP_DigestUpdate(mdctx, memories + j, PUBLIC_KEY_LENGTH);
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
                signWIF = encodeWIF(privateKey + i);
                encWIF = encodeWIF(privateKey + j);
                keu = formatKey(address, signWIF, encWIF);
                printf("%s\n", keu);
                fflush(stdout);
                free(address);
                free(signWIF);
                free(encWIF);
                free(keu);
            }
        }

        EVP_MD_CTX_free(mdctx);
    }

    free(memories);
    memories = NULL;
    free(privateKey);

    return 0;
}
