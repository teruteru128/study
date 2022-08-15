
#define _GNU_SOURCE 1
#include <err.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct kp
{
    unsigned char prikey[32];
    unsigned char pubkey[65];
};

int compar(const void *a, const void *b, void *arg)
{
    return memcmp(((struct kp *)a)->pubkey, ((struct kp *)b)->pubkey, 65);
}

int main(int argc, char const *argv[])
{
    char privatekeyfilename[PATH_MAX] = "";
    char publickeyfilename[PATH_MAX] = "";
    struct kp *list = malloc(sizeof(struct kp) * 16777216 * 8);
    FILE *privateKey = NULL;
    FILE *publicKey = NULL;
    int fail = 0;
    size_t i = 0;
    for (size_t filenum = 0; filenum < 8; filenum++)
    {
        snprintf(privatekeyfilename, PATH_MAX, "privateKeys%zu.bin", filenum);
        privateKey = fopen(privatekeyfilename, "rb");
        snprintf(publickeyfilename, PATH_MAX, "publicKeys%zu.bin", filenum);
        publicKey = fopen(publickeyfilename, "rb");
        if (privateKey == NULL || publicKey == NULL)
        {
            fail = 1;
            break;
        }
        for (i = 0; i < 16777216; i++)
        {
            if (fread(list[filenum * 16777216 + i].prikey, 32, 1, privateKey)
                != 1)
            {
                err(EXIT_FAILURE, "fread1");
            }
            if (fread(list[filenum * 16777216 + i].pubkey, 65, 1, publicKey)
                != 1)
            {
                err(EXIT_FAILURE, "fread2");
            };
        }
        fclose(privateKey);
        privateKey = NULL;
        fclose(publicKey);
        publicKey = NULL;
    }
    if (fail != 0)
    {
        free(list);
        err(EXIT_FAILURE, "error!");
    }
    printf("LOADED\n");
    qsort_r(list, 16777216 * 8, sizeof(struct kp), compar, NULL);

    struct stat st;

    if (stat("out", &st) != 0)
    {
        // out dirがなかったら作る
        mkdir("out", 0700);
    }

    for (size_t filenum = 0; filenum < 8; filenum++)
    {
        // 16777216件ずつファイルへ書き出し
        snprintf(privatekeyfilename, PATH_MAX, "out/privateKeys%zu.bin",
                 filenum);
        privateKey = fopen(privatekeyfilename, "wb");
        snprintf(publickeyfilename, PATH_MAX, "out/publicKeys%zu.bin",
                 filenum);
        publicKey = fopen(publickeyfilename, "wb");
        if (privateKey == NULL || publicKey == NULL)
        {
            fail = 1;
            break;
        }
        for (i = 0; i < 16777216; i++)
        {
            if (fwrite(list[filenum * 16777216 + i].prikey, 32, 1, privateKey)
                != 1)
            {
                err(EXIT_FAILURE, "fwrite1");
            }
            if (fwrite(list[filenum * 16777216 + i].pubkey, 65, 1, publicKey)
                != 1)
            {
                err(EXIT_FAILURE, "fwrite1");
            }
        }
        fclose(privateKey);
        privateKey = NULL;
        fclose(publicKey);
        publicKey = NULL;
    }

    free(list);
    return 0;
}
