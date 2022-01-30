
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <inttypes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <printint.h>
#define IN2_SIZE 24

static volatile atomic_size_t verifier;
static volatile sig_atomic_t eflag = 0;
static volatile int sig = 1;
static volatile int running = 1;

static void act(int siglocal)
{
    // fprintf(stderr, "%d,verifier=%" PRIu64 "\n", sig, verifier);
    sig = siglocal;
    eflag = 1;
}

#define IN                                                                    \
    "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/"                       \
    "zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s="

static void *function(void *arg)
{
    const EVP_MD *sha1 = EVP_sha1();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int *mdint = (unsigned int *)md;
    int i;
    char in2[IN2_SIZE] = "";
    uint64_t in1Length = strlen(IN);
    int verifierLength;
    for (;; verifier++)
    {
        EVP_DigestInit(ctx, sha1);
        EVP_DigestUpdate(ctx, IN, in1Length);
        verifierLength = snprintf(in2, IN2_SIZE, "%lu", verifier);
        // verifierLength = snprintUInt64(in2, IN2_SIZE, verifier);
        // ltoa(verifier, in2, 10);
        EVP_DigestUpdate(ctx, in2, (size_t)verifierLength);
        EVP_DigestFinal(ctx, md, NULL);
        if (mdint[0] == 0 && md[4] == 0)
        {
            printf("verifier : %" PRIu64 "\n", verifier);
            for (i = 0; i < SHA_DIGEST_LENGTH; i++)
            {
                printf("%02x", md[i]);
            }
            printf("\n");
            break;
        }
    }
    running = 0;
    EVP_MD_CTX_free(ctx);
    return NULL;
}

int main(void)
{
    FILE *fin = fopen("/dev/urandom", "rb");
    uint64_t seed = 0;
    fread(&seed, 8, 1, fin);
    fclose(fin);
    verifier = seed;

    if (signal(SIGINT, act) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }

    printf("pid : %d\n", getpid());

    pthread_t thread;
    int ret = pthread_create(&thread, NULL, function, NULL);

    while (running)
    {
        if(eflag)
        {
            fprintf(stderr, "%d,verifier=%" PRIu64 "\n", sig, verifier);
            eflag = 0;
        }
        usleep(250000);
    }

    pthread_join(thread, NULL);

    return 0;
}
