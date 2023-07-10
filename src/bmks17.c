
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

#define errchk(v, f)                                                          \
    if (!v)                                                                   \
    {                                                                         \
        unsigned long err = ERR_get_error();                                  \
        fprintf(stderr, #f " : %s\n", ERR_error_string(err, NULL));           \
        return EXIT_FAILURE;                                                  \
    }

static int load(unsigned char *buf, const char *infile)
{
    int fd = open(infile, O_RDONLY);
    if (fd < 0)
    {
        return 1;
    }
    ssize_t d = read(fd, buf, 1090519040UL);
    if (d < 1090519040UL)
    {
        close(fd);
        return 1;
    }
    close(fd);
    return 0;
}

struct arg
{
    unsigned char *keys;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void initRandom(struct drand48_data *buffer)
{
    unsigned short seed[3];
    pthread_mutex_lock(&mutex);
    ssize_t ret = getrandom(seed, 6, 0);
    pthread_mutex_unlock(&mutex);
    if (ret != 6)
    {
        perror("getrandom");
        return;
    }
    seed48_r(seed, buffer);
}

static int next(struct drand48_data *buffer, int bits)
{
    long r;
    mrand48_r(buffer, &r);
    return ((unsigned int)r) >> (32 - bits);
}

static int nextInt(struct drand48_data *rnd, int bound)
{
    if (bound <= 0)
        return 0;

    int r = next(rnd, 31);
    // mask
    int m = bound - 1;
    if ((bound & m) == 0)
    {
        r = (uint32_t)((bound * (uint64_t)r) >> 31);
    }
    else
    { // reject over-represented candidates
        for (int u = r; u - (r = u % bound) + m < 0; u = next(rnd, 31))
            ;
    }
    return r;
}

static void *func(void *a)
{
    const struct arg *arg = (struct arg *)a;
    unsigned char *keys = arg->keys;
    struct drand48_data buffer = { 0 };
    initRandom(&buffer);
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *sha512ctx1 = EVP_MD_CTX_new();
    EVP_MD_CTX *sha512ctx2 = EVP_MD_CTX_new();
    EVP_MD_CTX *ripemd160ctx = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_DigestInit_ex2(sha512ctx2, sha512, NULL);
#else
    EVP_DigestInit_ex(sha512ctx2, sha512, NULL);
#endif
    unsigned char hash[EVP_MAX_MD_SIZE];
    size_t j = 0;
    unsigned char *enckey = NULL;
    int first = 1;
    for (size_t sigindex = nextInt(&buffer, 67108864); sigindex < 67108864;
         sigindex++)
    {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(sha512ctx1, sha512, NULL);
#else
        EVP_DigestInit_ex(sha512ctx1, sha512, NULL);
#endif
        EVP_DigestUpdate(sha512ctx1, keys + ((sigindex << 6) + sigindex), 65);
        for (j = 0, enckey = keys; j < 67108864UL; j++, enckey += 65)
        {
            EVP_MD_CTX_copy_ex(sha512ctx2, sha512ctx1);
            EVP_DigestUpdate(sha512ctx2, enckey, 65);
            EVP_DigestFinal_ex(sha512ctx2, hash, NULL);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
            EVP_DigestInit_ex2(ripemd160ctx, ripemd160, NULL);
#else
            EVP_DigestInit_ex(ripemd160ctx, ripemd160, NULL);
#endif
            EVP_DigestUpdate(ripemd160ctx, hash, 64);
            EVP_DigestFinal_ex(ripemd160ctx, hash, NULL);
            if ((*(unsigned long *)hash) & 0x0000ffffffffffffUL)
            {
                continue;
            }
            for (size_t i = 0; i < 65; i++)
            {
                printf("%02x", keys[((sigindex << 6) + sigindex) + i]);
            }
            fputs(",", stdout);
            for (size_t i = 0; i < 65; i++)
            {
                printf("%02x", enckey[i]);
            }
            fputs("\n", stdout);
        }
        fprintf(stderr, "%zu: done\n", sigindex);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *def = OSSL_PROVIDER_load(NULL, "default");
    errchk(def, OSSL_PROVIDER_load);
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    errchk(legacy, OSSL_PROVIDER_load);
#endif
    unsigned char *keys = malloc(16777216UL * 65 * 4);
    const char *f[] = { "/mnt/d/keys/public/publicKeys0.bin",
                        "/mnt/d/keys/public/publicKeys1.bin",
                        "/mnt/d/keys/public/publicKeys2.bin",
                        "/mnt/d/keys/public/publicKeys3.bin" };
    int ret = 0;
    for (size_t i = 0; i < 4; i++)
    {
        ret = load(keys + 1090519040UL * i, f[i]);
        if (ret != 0)
        {
            perror("a");
            return 1;
        }
    }

    struct arg arg = { keys };

    size_t count = 4;

    if (argc >= 2)
    {
        count = strtoul(argv[1], NULL, 10);
    }

    pthread_t *t = alloca(sizeof(pthread_t) * count);
    for (size_t i = 0; i < count; i++)
    {
        ret = pthread_create(t + i, NULL, func, &arg);
        if (ret != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }
    for (size_t i = 0; i < count; i++)
    {
        pthread_join(t[i], NULL);
    }

    free(keys);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER_unload(def);
    OSSL_PROVIDER_unload(legacy);
#endif
    return 0;
}
