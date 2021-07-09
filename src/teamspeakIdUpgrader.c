
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <inttypes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <printint.h>
#define IN2_SIZE 24

static volatile uint64_t verifier;

static void act(int sig)
{
    fprintf(stderr, "%d,verifier=%" PRIu64 "\n", sig, verifier);
}

#define IN "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/" \
          "zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s="

int main(void)
{
    const EVP_MD *sha1 = EVP_sha1();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char md[EVP_MAX_MD_SIZE];
    int i;
    char in2[IN2_SIZE] = "";
    uint64_t in1Length = strlen(IN);
    int verifierLength;

    FILE *fin = fopen("/dev/urandom", "rb");
    uint64_t seed = 0;
    fread(&seed, 1, 8, fin);
    fclose(fin);
    verifier = seed;

    if (signal(SIGINT, act) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }

    printf("%d\n", getpid());
    for (;; verifier++)
    {
        EVP_DigestInit(ctx, sha1);
        EVP_DigestUpdate(ctx, IN, in1Length);
        verifierLength = snprintf(in2, IN2_SIZE, "%lu", verifier);
        // verifierLength = snprintUInt64(in2, IN2_SIZE, verifier);
        // ltoa(verifier, in2, 10);
        EVP_DigestUpdate(ctx, in2, (size_t)verifierLength);
        EVP_DigestFinal(ctx, md, NULL);
        if (md[0] == 0 && md[1] == 0 && md[2] == 0 && md[3] == 0 && md[4] == 0)
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
    EVP_MD_CTX_free(ctx);
    return 0;
}
