
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/time.h>

#include <printint.h>
#define IN2_SIZE 20

#define CTX_SIZE 8

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

#define IN1 "MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0"

int main(int argc, char **argv)
{
    SHA_CTX ctx[CTX_SIZE];
    char buf[CTX_SIZE][20];
    unsigned char md[CTX_SIZE][SHA_DIGEST_LENGTH];
    uint64_t counter[CTX_SIZE];
    size_t in1Length = strlen(IN1);
    size_t counterLength[CTX_SIZE];
    size_t i = 0;
    size_t j = 0;

    initRandom();

    for (i = 0; i < CTX_SIZE; i++)
    {
        counter[i] = getRandomU64();
    }
    while (1)
    {
        for (i = 0; i < CTX_SIZE; i++)
        {
            counter[i]++;
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            SHA1_Init(&(ctx[i]));
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            SHA1_Update(&(ctx[i]), IN1, in1Length);
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            counterLength[i] = snprintUInt64(buf[i], 20, counter[i]);
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            SHA1_Update(&(ctx[i]), buf[i], counterLength[i]);
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            SHA1_Final(md[i], &(ctx[i]));
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            if (md[i][0] == 0 && md[i][1] == 0 && md[i][2] == 0 && md[i][3] == 0 && md[i][4] == 0)
            {
                printf("verifier : %lu\n", counter[i]);
                for (j = 0; j < SHA_DIGEST_LENGTH; j++)
                {
                    printf("%02x", md[i][j]);
                }
                printf("\n");
                if (md[i][0] == 0 && md[i][1] == 0 && md[i][2] == 0 && md[i][3] == 0 && md[i][4] == 0
                 && md[i][5] == 0 && md[i][6] == 0 && md[i][7] == 0 && md[i][8] == 0 && md[i][9] == 0
                 && md[i][10] == 0 && md[i][11] == 0 && md[i][12] == 0 && md[i][13] == 0 && md[i][14] == 0 
                 && md[i][15] == 0 && md[i][16] == 0 && md[i][17] == 0 && md[i][18] == 0 && md[i][19] == 0)
                {
                    // 全ビット0
                    goto out;
                }
            }
        }
    }
out:
    return 0;
}
