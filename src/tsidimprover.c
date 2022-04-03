
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <arpa/inet.h>
#include <inttypes.h>
#include <limits.h>
#include <netinet/in.h>
#include <numsys.h>
#include <openssl/sha.h>
#include <printint.h>
#include <random.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/time.h>
#include <time.h>

#define IN2_SIZE 20
#define CTX_SIZE 1

/** * teamspeak * */
static void initRandom()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand48(((tv.tv_usec & 0xFFFFFFFF) << 32)
            ^ ((tv.tv_usec >> 32) & 0xFFFFFFFF) ^ tv.tv_sec);
}

static uint64_t getRandomU64()
{
    return ((uint64_t)mrand48() << 33)
           ^ (((uint64_t)mrand48() << 16) & 0xffffffffULL)
           ^ (((uint64_t)mrand48()) & 0xffffffffULL);
}

// main id :
// MEwDAgcAAgEgAiEA7Vo1+Orf2xuuu6hTPAPldSfrUZZ7WYAzpRcO5DoYFLoCIF1JKVBctOGvMOy495O/BWFuFEYH4i1f6vU0b9+a64RD
// android id :
// MEwDAgcAAgEgAiBK4dcDZUSLCxmvRfMWMAQf1JzSrLzZakLqDsULzT28OwIhAILbBS66JoN1Xo2YsC1xDPDhukJjVO2guoeL+AM27Vfn
// 3rd id :
// MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=
// new id :
// MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0

#define IN1                                                                   \
    "MEwDAgcAAgEgAiBK4dcDZUSLCxmvRfMWMAQf1JzSrLzZakLqDsULzT28OwIhAILbBS66JoN" \
    "1Xo2YsC1xDPDhukJjVO2guoeL+AM27Vfn"

static int calcSecurityLevel(SHA_CTX *ctx, unsigned char *md, char *buf,
                             const char *id, size_t idlen, uint64_t counter)
{
    int i = 0;
    int j = 0;
    SHA1_Init(ctx);
    SHA1_Update(ctx, id, idlen);
    size_t counterLength = (size_t)snprintf(buf, 25, "%lu", counter);
    SHA1_Update(ctx, buf, counterLength);
    SHA1_Final(md, ctx);
    for (i = 0; md[i] == 0 && i < SHA_DIGEST_LENGTH; i++)
    {
    }
    if (i < SHA_DIGEST_LENGTH)
        j = numberOfTrailingZeros(md[i] & 0xff);
    return (i << 3) + j;
}

int main(int argc, char **argv)
{
    SHA_CTX ctx;
    unsigned char md[SHA_DIGEST_LENGTH];
    char buf[25];
    const size_t in1Length = strlen(IN1);
    size_t i = 0;
    int64_t securityLevel = 0;
    int64_t maxSecurityLevel = 39;

    initRandom();

    for (uint64_t counter = 0x8000000000UL; counter < 0x10000000000UL;
         counter++)
    {
        securityLevel
            = calcSecurityLevel(&ctx, md, buf, IN1, in1Length, counter);
        if (maxSecurityLevel <= securityLevel)
        {
            printf("Max update: %ld -> %ld, counter : %lu\n", maxSecurityLevel,
                   securityLevel, counter);
            maxSecurityLevel = securityLevel;
        }
        if (securityLevel >= 40)
        {
            printf("counter(%" PRIu64 ") : %24" PRIu64 ", ", securityLevel,
                   counter);
            for (i = 0; i < SHA_DIGEST_LENGTH; i++)
            {
                printf("%02x", md[i]);
            }
            printf("\n");
        }
    }
    return EXIT_SUCCESS;
}
