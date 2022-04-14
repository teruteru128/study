
#include <inttypes.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HEXTABLE "0123456789abcdef"

int main(int argc, char const *argv[])
{
    unsigned char *publickeys = calloc(67108864UL, 65UL);
    {
        FILE *fin = fopen("publicKeys.bin", "rb");
        if (fin == NULL)
        {
            return 1;
        }
        if (fread(publickeys, 65, 67108864, fin) != 67108864)
        {
            perror("fread");
            fclose(fin);
            free(publickeys);
            return 1;
        }
        fclose(fin);
    }
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;
    unsigned char hashwork[EVP_MAX_MD_SIZE] = "";
    unsigned char *signpubkey = NULL;
    struct timespec ts = { 0 };
    struct tm machine_tm = { 0 };
    char datetime[BUFSIZ] = "";
    fputs("start\n", stdout);
    for (i = 0; i < 4362076160UL; i += 65)
    {
        signpubkey = publickeys + i;
        for (j = 0; j < 4362076160UL; j += 65)
        {
            EVP_DigestInit(mdctx, sha512);
            EVP_DigestUpdate(mdctx, signpubkey, 65);
            EVP_DigestUpdate(mdctx, publickeys + j, 65);
            EVP_DigestFinal(mdctx, hashwork, NULL);
            EVP_DigestInit(mdctx, ripemd160);
            EVP_DigestInit(mdctx, ripemd160);
            EVP_DigestUpdate(mdctx, hashwork, 64);
            EVP_DigestFinal(mdctx, hashwork, NULL);
#if __BYTE_ORDER == __LITTLE_ENDIAN
            if (((*(uint64_t *)hashwork) & 0x000000ffffffffffUL) == 0UL)
#elif __BYTE_ORDER == __BIG_ENDIAN
            if (((*(uint64_t *)hashwork) & 0xffffffffff000000UL) == 0UL)
#else
#error "byte order error!"
#endif
            {
                for (k = 0; k < 20; k++)
                {
                    fputc(HEXTABLE[(hashwork[k] >> 4) & 0x0f], stdout);
                    fputc(HEXTABLE[(hashwork[k] >> 0) & 0x0f], stdout);
                }
                printf(", %zu, %zu\n", i / 65, j / 65);
            }
        }
        clock_gettime(CLOCK_REALTIME, &ts);
        localtime_r(&ts.tv_sec, &machine_tm);
        strftime(datetime, BUFSIZ, "%EC%Ey%B%d日 %X %EX", &machine_tm);
        printf("i: %10zu終わり(%s)\n", i / 65, datetime);
    }

    EVP_MD_CTX_free(mdctx);
    free(publickeys);
    return 0;
}
