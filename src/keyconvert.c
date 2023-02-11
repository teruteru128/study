
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

/* 秘密鍵を公開鍵に変換する */
int func1()
{
    char inputpath[PATH_MAX];
    char outputpath[PATH_MAX];
    int prikeyfd = -1;
    int pubkeyfd = -1;
    int trimmedpubkeyfd = -1;
    FILE *prikeyf = NULL;
    FILE *pubkeyf = NULL;
    FILE *trimmedpubkeyf = NULL;
    EC_GROUP *secp256k1 = EC_GROUP_new_by_curve_name(NID_secp256k1);
    BN_CTX *ctx = NULL;
    BIGNUM *prikeybn = NULL;
    EC_POINT *pubkeyp = NULL;
    unsigned char *prikeymap = NULL;
    unsigned char *pubkeymap
        = mmap(NULL, 16777216 * 65, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pubkeymap == MAP_FAILED)
    {
        perror("mmap read");
        return 1;
    }
    unsigned char *trimmedpubkeymap
        = mmap(NULL, 16777216 * 64, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trimmedpubkeymap == MAP_FAILED)
    {
        perror("mmap write");
        return 1;
    }
    ssize_t d = 0;
    size_t j = 0;
    for (size_t i = 8; i < 256; i++)
    {
        // open private key file
        snprintf(inputpath, PATH_MAX, "/mnt/d/keys/private/privateKeys%zu.bin", i);
        prikeyfd = open(inputpath, O_RDONLY);
        if (prikeyfd < 0)
        {
            perror("open read");
            return 1;
        }
        // 秘密鍵ファイルをメモリーにマッピング
        prikeymap
            = mmap(NULL, 16777216 * 32, PROT_READ, MAP_PRIVATE, prikeyfd, 0);
        if (prikeymap == MAP_FAILED)
        {
            perror("mmap read");
            return 1;
        }
        close(prikeyfd);
#pragma omp parallel default(none)                                            \
    shared(prikeymap, secp256k1, trimmedpubkeymap,                            \
           pubkeymap) private(ctx, j, prikeybn, pubkeyp)
        {
            ctx = BN_CTX_new();
            BN_CTX_start(ctx);
            prikeybn = BN_CTX_get(ctx);
            pubkeyp = EC_POINT_new(secp256k1);
#pragma omp for
            for (j = 0; j < 16777216; j++)
            {
                BN_bin2bn(prikeymap + j * 32, 32, prikeybn);
                EC_POINT_mul(secp256k1, pubkeyp, prikeybn, NULL, NULL, ctx);
                EC_POINT_point2oct(secp256k1, pubkeyp,
                                   POINT_CONVERSION_UNCOMPRESSED,
                                   pubkeymap + 65 * j, 65, ctx);
            }
            BN_CTX_end(ctx);
            BN_CTX_free(ctx);
            EC_POINT_free(pubkeyp);
        }
        for (j = 0; j < 16777216; j++)
        {
            memcpy(trimmedpubkeymap + 64 * j, pubkeymap + 65 * j + 1, 64);
        }
        snprintf(outputpath, PATH_MAX, "/mnt/d/keys/public/publicKeys%zu.bin", i);
        pubkeyf = fopen(outputpath, "wb");
        if (pubkeyf == NULL)
        {
            perror("fopen pubkeyf");
            return 1;
        }
        if (fwrite(pubkeymap, 65, 16777216, pubkeyf) < 16777216)
        {
            perror("write");
        }
        fclose(pubkeyf);
        snprintf(outputpath, PATH_MAX,
                 "/mnt/d/keys/public/trimmed/publicKeys%zu.bin", i);
        trimmedpubkeyf = fopen(outputpath, "wb");
        if (trimmedpubkeyf == NULL)
        {
            perror("fopen trimmedpubkeyf");
            return 1;
        }
        if (fwrite(trimmedpubkeymap, 64, 16777216UL, trimmedpubkeyf)
            < 16777216)
        {
            perror("write trimmedpubkeymap");
        }
        if (fclose(trimmedpubkeyf) == -1)
        {
            perror("close");
        }
        printf("%s done.\n", inputpath);
        munmap(prikeymap, 16777216 * 32);
    }
    munmap(pubkeymap, 16777216 * 65);
    munmap(trimmedpubkeymap, 16777216 * 64);
    EC_GROUP_free(secp256k1);
    return 0;
}

void printhelp(char *argv0) { fprintf(stderr, "%s code option\n", argv0); }

/* null scanner */
int func2(int argc, char **argv)
{
    if (argc < 1)
    {
        return 1;
    }
    char rpath[PATH_MAX] = "";
    strncpy(rpath, argv[0], PATH_MAX);
    int fd = open(rpath, O_RDONLY);
    unsigned char *m
        = mmap(NULL, 16777216UL * 32, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    const size_t l = argc >= 2 ? strtol(argv[1], NULL, 10) : 32;
    unsigned char mam[l];
    memset(mam, 0, l);
    unsigned char *m2 = memmem(m, 16777216UL * 32, mam, l);
    if (m2 != NULL)
    {
        printf("0x%016lx, %ld\n", m2 - m, m2 - m);
    }
    munmap(m, 16777216 * 32);
    return 0;
}

int main(int argc, char const *argv[])
{
    int code = -1;
    if (argc >= 2)
    {
        code = strtol(argv[1], NULL, 0);
    }
    switch (code)
    {
    case 1:
        func1();
        break;

    case 2:
        func2(argc - 1, argv + 2);
        break;

    default:
        printhelp(argv[0]);
        break;
    }
    return 0;
}
