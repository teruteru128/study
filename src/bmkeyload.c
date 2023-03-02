
#include <stdio.h>
#include <bitmessage_type.h>

static int loadKey1(unsigned char *publicKey, const char *path, size_t size,
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

int loadPublicKey(PublicKey *publicKey, const char *path)
{
    // public keyは頻繁に使うのでメモリに読み込んでおく
    return loadKey1((unsigned char *)publicKey, path, 65, 16777216);
}
