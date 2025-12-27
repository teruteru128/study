
#include <stdio.h>
#include <stdlib.h>

int loadKey1(unsigned char *publicKey, const char *path, size_t size,
                    size_t num)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    FILE *publicKeyFile = fopen(path, "rb");
    if (publicKeyFile == NULL)
    {
        if (publicKeyFile != NULL)
        {
            fclose(publicKeyFile);
        }
        return 1;
    }
    size_t pubnum = fread(publicKey, size, num, publicKeyFile);
    fclose(publicKeyFile);
    if (pubnum != num)
    {
        perror("fread");
        free(publicKey);
        return 1;
    }
    return 0;
}

int loadPrivateKey1(unsigned char *publicKey, const char *path)
{
    return loadKey1(publicKey, path, 32, 16777216);
}

int loadPublicKey(unsigned char *publicKey, const char *path)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    return loadKey1(publicKey, path, 64, 16777216);
}
