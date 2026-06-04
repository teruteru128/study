
#include <assert.h>
#include <bitmessage.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int calchash(EVP_MD_CTX *mdctx, const EVP_MD *sha512,
                    const EVP_MD *ripemd160, unsigned char *ekey, unsigned char *skey,
                    size_t *casted)
{
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned int mdlen = 0;
    unsigned long tmp = 0;

    EVP_DigestInit(mdctx, sha512);
    EVP_DigestUpdate(mdctx, ekey, 65);
    EVP_DigestUpdate(mdctx, skey, 65);
    EVP_DigestFinal(mdctx, hash, &mdlen);
    assert(mdlen == 64);
    EVP_DigestInit(mdctx, ripemd160);
    EVP_DigestUpdate(mdctx, hash, 64);
    EVP_DigestFinal(mdctx, hash, &mdlen);
    assert(mdlen == 20);
    *casted = htobe64(*(unsigned long *)hash);
    return 0;
}

/**
 * @brief sanbox func.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    (void)argc, (void)argv;
    int ret = EXIT_SUCCESS;

    FILE *fin = fopen("publicKeys.bin", "rb");

    unsigned char key1[63 * 65] = "";
    unsigned char key2[63 * 65] = "";

    size_t t = fread(key1, 65, 63, fin);

    if (t < 63)
    {
        perror("fread");
        fclose(fin);
        return EXIT_FAILURE;
    }

    fclose(fin);

    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned int mdlen = 0;
    unsigned long tmp = 0;

    size_t counts[21] = { 0 };

    for (size_t i = 0; i < 4095; i += 65)
    {
        for (size_t j = 0; j < 4095; j += 65)
        {
            // TODO: 関数化
            calchash(mdctx, sha512, ripemd160, key1 + i, key1 + j, &tmp);
            counts[((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3]++;
        }
    }
    //(void)fseek(fin, 63L * 0, SEEK_SET);
    t = fread(key2, 65, 63, fin);
    for (size_t i = 0; i < 4095; i += 65)
    {
        for (size_t j = 0; j < 4095; j += 65)
        {
            calchash(mdctx, sha512, ripemd160, key1 + i, key2 + j, &tmp);
            counts[((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3]++;
        }
    }
    fclose(fin);
    for (size_t i = 0; i < 21; i++)
    {
        printf("%zu\n", counts[i]);
    }

    return ret;
}
