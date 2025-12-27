
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <endian.h>
#include <openssl/evp.h>
#include <sys/random.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
atomic_int isDone = 0;
double diff;
uint8_t result[16];
uint8_t resulthash[16];

struct arg
{
    int count;
    char prefix[8];
};

void *task(void *arg)
{
    uint64_t i = (uint64_t)((struct arg *)arg)->count << 32;
    uint8_t input[16];
    memcpy(input, ((struct arg *)arg)->prefix, 8);
    EVP_MD *md5 = EVP_MD_fetch(NULL, "MD5", NULL);
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    uint8_t *tail = input + 8;
    uint8_t hash[EVP_MAX_MD_SIZE];
    time_t start;
    time_t stop;
    start = time(NULL);
    size_t len = 0;
    size_t size = 8;
    ssize_t bytes_read;
    while (len < size)
    {
        bytes_read = getrandom(tail + len, 8, 0);
        len += bytes_read;
    }
    while (!isDone)
    {
        EVP_DigestInit_ex2(ctx, md5, NULL);
        EVP_DigestUpdate(ctx, input, 16);
        EVP_DigestFinal_ex(ctx, hash, NULL);
        if ((be64toh(*(uint64_t *)hash) & 0xFFFFFFFF00000000UL) == i)
        {
            break;
        }
        (*(uint64_t *)tail)++;
    }
    stop = time(NULL);
    pthread_mutex_lock(&lock);
    if (!isDone)
    {
        isDone = 1;
        memcpy(result, input, 16);
        memcpy(resulthash, hash, 16);
        diff = difftime(stop, start);
    }
    pthread_mutex_unlock(&lock);
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(md5);
    return NULL;
}

#define THREADS 15

int main(int argc, char const *argv[])
{
    time_t start;
    time_t stop;
    pthread_t threads[THREADS];
    struct arg arg;
    size_t len = 0;
    size_t size = 8;
    ssize_t bytes_read;
    while (len < size)
    {
        bytes_read = getrandom(arg.prefix + len, 8, 0);
        len += bytes_read;
    }
    for (int i = 0; i < 16; i++)
    {
        arg.count = i;
        for (int j = 0; j < THREADS; j++)
        {
            pthread_create(&threads[j], NULL, task, &arg);
        }
        for (int j = 0; j < THREADS; j++)
        {
            pthread_join(threads[j], NULL);
        }
        printf("%016" PRIx64 ": ", be64toh(*((uint64_t *)resulthash)));
        for (int j = 0; j < 16; j++)
        {
            printf("%02" PRIx8, result[j]);
        }
        printf(", %f\n", diff);
        isDone = 0;
    }
    return 0;
}
