
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/time.h>
#include <limits.h>
#include "printint.h"
#include "random.h"

#define IN2_SIZE 20
#define CTX_SIZE 1

/** * teamspeak * */
static void initRandom()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand48(((tv.tv_usec & 0xFFFFFFFF) << 32) ^ ((tv.tv_usec >> 32) & 0xFFFFFFFF) ^ tv.tv_sec);
}

static uint64_t getRandomU64()
{
    return ((uint64_t)mrand48() << 33) ^ (((uint64_t)mrand48() << 16) & 0xffffffffULL) ^ (((uint64_t)mrand48()) & 0xffffffffULL);
}

// 3rd id : MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=
// main id : MEwDAgcAAgEgAiEA7Vo1+Orf2xuuu6hTPAPldSfrUZZ7WYAzpRcO5DoYFLoCIF1JKVBctOGvMOy495O/BWFuFEYH4i1f6vU0b9+a64RD
// android id : MEwDAgcAAgEgAiBK4dcDZUSLCxmvRfMWMAQf1JzSrLzZakLqDsULzT28OwIhAILbBS66JoN1Xo2YsC1xDPDhukJjVO2guoeL+AM27Vfn
// new id : MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0

#define IN1 "MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0"

static int numberOfLeadingZeros(unsigned int i)
{
    if(i <= 0)
    {
        return i == 0 ? 32:0;
    }
    int n = 32;
        if (i >= 1 << 16) { n -= 16; i >>= 16; }
        if (i >= 1 <<  8) { n -=  8; i >>=  8; }
        if (i >= 1 <<  4) { n -=  4; i >>=  4; }
        if (i >= 1 <<  2) { n -=  2; i >>=  2; }
        return n - (i >> 1);
}

static int numberOfTrailingZeros(unsigned int i)
{
    i = ~i & (i - 1);
    if(i <= 0) return i & 32;
    int n = 1;
    if (i > 1 << 16) { n += 16; i >>= 16; }
    if (i > 1 <<  8) { n +=  8; i >>=  8; }
    if (i > 1 <<  4) { n +=  4; i >>=  4; }
    if (i > 1 <<  2) { n +=  2; i >>=  2; }
    return n + (i >> 1);
}

static int calcSecurityLevel(unsigned char *md, char *id, size_t idlen, uint64_t counter, SHA_CTX *ctx)
{
    char buf[22];
    size_t counterLength = 0;
    int i = 0;
    int j = 0;
    SHA1_Init(ctx);
    SHA1_Update(ctx, id, idlen);
    counterLength = snprintUInt64(buf, 25, counter);
    SHA1_Update(ctx, buf, counterLength);
    SHA1_Final(md, ctx);
    for (i = 0; md[i] == 0 && i < SHA_DIGEST_LENGTH; i++)
    {
    }
    if(i < SHA_DIGEST_LENGTH)
    j = numberOfTrailingZeros(md[i] & 0xff);
    return (i << 3) + j;
}

int main(int argc, char **argv)
{
    SHA_CTX ctx;
    unsigned char md[SHA_DIGEST_LENGTH];
    const size_t in1Length = strlen(IN1);
    size_t i = 0;
    size_t j = 0;
    int64_t securityLevel = 0;
    int64_t maxSecurityLevel = 0;

    initRandom();
    //nextBytes((char *)&counter, sizeof(uint64_t));
    //counter = counter & 0xffffffffffffUL;

    for (uint64_t counter = 27579080; counter <= UINT_MAX; counter++)
    {
        securityLevel = calcSecurityLevel(md, IN1, in1Length, counter, &ctx);
        if (maxSecurityLevel < securityLevel)
        {
            printf("Max update: %" PRId64 " -> %" PRId64 "\n", maxSecurityLevel, securityLevel);
            maxSecurityLevel = securityLevel;
        }
        if (securityLevel >= 28)
        {
            printf("verifier(%" PRIu64 ") : %24" PRIu64 ", ", securityLevel, counter);
            for (j = 0; j < SHA_DIGEST_LENGTH; j++)
            {
                printf("%02x", md[j]);
            }
            printf("\n");
        }
    }
out:
    return EXIT_SUCCESS;
}
