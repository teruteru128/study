
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <fcntl.h>
#include <limits.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

int main(int argc, char const *argv[])
{
    char inputpath[PATH_MAX];
    char outputpath[PATH_MAX];
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    unsigned char rawpubkey[65];
    FILE *outputf = NULL;
    int inputfd = -1;
    unsigned char *prikey = NULL;
    unsigned char *pubkey = mmap(NULL, 1090519040, PROT_READ | PROT_WRITE,
                                 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    for (size_t i = 170; i < 256; i++)
    {
        snprintf(inputpath, PATH_MAX, "/mnt/d/keys/private/privateKeys%zu.bin",
                 i);
        snprintf(outputpath, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin",
                 i);
        inputfd = open(inputpath, O_RDONLY | O_CLOEXEC);
        if (inputfd < 0)
        {
            perror("open");
            break;
        }
        prikey = mmap(NULL, 536870912, PROT_READ, MAP_PRIVATE, inputfd, 0);
        close(inputfd);
        if (prikey == (void *)-1 || pubkey == (void *)-1)
        {
            perror("mmap");
            break;
        }
#pragma omp parallel default(none) shared(secp256k1, prikey, pubkey)
        {
            BN_CTX *ctx = BN_CTX_new();
            BN_CTX_start(ctx);
            BIGNUM *prikeybn = BN_CTX_get(ctx);
            EC_POINT *pubkeyp = EC_POINT_new(secp256k1);
#pragma omp for
            for (size_t j = 0; j < 16777216; j++)
            {
                BN_bin2bn(prikey + (j << 5), 32, prikeybn);
                EC_POINT_mul(secp256k1, pubkeyp, prikeybn, NULL, NULL, ctx);
                EC_POINT_point2oct(secp256k1, pubkeyp,
                                   POINT_CONVERSION_UNCOMPRESSED,
                                   pubkey + ((j << 6) + j), 65, ctx);
            }
            BN_CTX_end(ctx);
            BN_CTX_free(ctx);
            EC_POINT_free(pubkeyp);
        }
        outputf = fopen(outputpath, "w");
        if (outputf == NULL)
        {
            perror("fopen");
            break;
        }
        fwrite(pubkey, 65, 16777216, outputf);
        munmap(prikey, 536870912);
        fclose(outputf);
        printf("%zu終わり\n", i);
        memset(inputpath, 0, PATH_MAX);
        memset(outputpath, 0, PATH_MAX);
    }
    munmap(pubkey, 1090519040);
    EC_GROUP_free(secp256k1);
    return 0;
}
