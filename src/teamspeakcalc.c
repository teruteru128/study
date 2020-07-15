
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#include <printint.h>
#include <java_random.h>
#define IN2_SIZE 21

typedef union mc
{
    unsigned char md[SHA_DIGEST_LENGTH];
    uint32_t ddd[SHA_DIGEST_LENGTH / sizeof(uint32_t)];
} MC;

/**
 * 
 */
int main(int argc, char **argv)
{
    const EVP_MD *sha1 = EVP_sha1();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char md[SHA_DIGEST_LENGTH];
    int i;
    char in1[125] = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    uint64_t in1Length = strlen(in1);
    uint64_t verifier;
    size_t verifierLength;
    int64_t rnd = 0;

    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int64_t seed = (tv.tv_usec & 0x000FFFFF) << 28;
        seed ^= tv.tv_sec;
        printf("%016lx\n", seed);
        printf("%ld.%06ld\n", tv.tv_sec, tv.tv_usec);
        rnd = initialScramble(seed);
    }
    /*
        1111111111111111111111111111111100000000000000000000000000000000
        0000000000000000111111111111111111111111111111110000000000000000
        0000000000000000000000000000000011111111111111111111111111111111
    */
    verifier = nextLong(&rnd);
    for (;; verifier++)
    {
        EVP_DigestInit(ctx, sha1);
        verifierLength = snprintf(&in1[104], IN2_SIZE, "%ld", verifier);
        EVP_DigestUpdate(ctx, in1, in1Length + verifierLength);
        EVP_DigestFinal(ctx, md, NULL);
        if (memcmp(md, "\0\0\0\0\0", 5) == 0)
        {
            printf(_("verifier : %" PRIu64 "\n"), verifier);
            for (i = 0; i < SHA_DIGEST_LENGTH; i++)
            {
                printf("%02x", md[i]);
            }
            printf("\n");
            break;
        }
    }
    /* DEAD CODE ***********************/
    EVP_MD_CTX_free(ctx);
    return EXIT_SUCCESS;
}
