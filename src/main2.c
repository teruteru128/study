
#define _GNU_SOURCE
#include "config.h"

#include <err.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

struct kp
{
    unsigned char prikey[32];
    unsigned char pubkey[65];
};

int compar(const void *a, const void *b, void *c)
{
    return memcmp(((struct kp *)a)->pubkey, ((struct kp *)b)->pubkey, 65);
}

int hiho(int argc, char **argv, const char **envp)
{
#if 0
    char privatekeyfilename[PATH_MAX] = "";
    char publickeyfilename[PATH_MAX] = "";
    struct kp *list = malloc(sizeof(struct kp) * 16777216 * 8);
    FILE *privateKey = NULL;
    FILE *publicKey = NULL;
    int fail = 0;
    for (size_t filenum = 0; filenum < 8; filenum++)
    {
        snprintf(privatekeyfilename, PATH_MAX, "privateKeys%zu.bin", filenum);
        privateKey = fopen(privatekeyfilename, "rb");
        snprintf(publickeyfilename, PATH_MAX, "publicKeys%zu.bin", filenum);
        privateKey = fopen(publickeyfilename, "rb");
        if (privateKey == NULL || publicKey == NULL)
        {
            fail = 1;
            break;
        }
        for (size_t i = 0; i < 16777216; i++)
        {
            fread(list[filenum * 16777216 + i].prikey, 32, 1, privateKey);
            fread(list[filenum * 16777216 + i].pubkey, 65, 1, publicKey);
        }
        fclose(privateKey);
        fclose(publicKey);
    }
    if (fail != 0)
    {
        err(EXIT_FAILURE, "");
    }

    qsort_r(list, 16777216 * 8, sizeof(struct kp), compar, NULL);

    for (size_t filenum = 0; filenum < 8; filenum++)
    {
        // 16777216件ずつファイルへ書き出し
    }

    free(list);
#endif
    char *tail = NULL;
    char envkey[64] = "";
    size_t length = 0;
    for (size_t i = 0; envp[i] != NULL; i++)
    {
        tail = strchr(envp[i], '=');
        if (tail == NULL)
        {
            continue;
        }
        length = MIN(tail - envp[i], 64);
        strncpy(envkey, envp[i], length);
        envkey[length] = '\0';
        printf("%s\n", envkey);
    }
    fputs("--\n", stdout);
    printf("%s, %s, %d\n", getenv("_"), argv[0], strcmp(getenv("_"), argv[0]));
    char *a = getenv("OLDPWD");
    printf("%s\n", a);
    return 0;
}
