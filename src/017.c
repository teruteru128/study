
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

#define IN1 "MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0"

int calcSecurityLevel(unsigned char *md, char* id, uint64_t counter){
  SHA_CTX ctx;
  char buf[22];
  size_t counterLength = 0;
  int i = 0;
  int j = 0;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, id, strlen(id));
  counterLength = snprintUInt64(buf, 20, counter);
  SHA1_Update(&ctx, buf, counterLength);
  SHA1_Final(md, &ctx);
  for(i = 0; md[i] == 0&&i < SHA_DIGEST_LENGTH; i++){
  
  }
  for(j = 0; (md[i] >> j) & 0x01 == 0; j++){}
  return i << 3 + j;
}

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
    int64_t securityLevel = 0;
    int64_t maxSecurityLevel = 0;

    initRandom();

    for (i = 0; i < CTX_SIZE; i++)
    {
        counter[i] = getRandomU64();
    }
    while (maxSecurityLevel < 160)
    {
        for (i = 0; i < CTX_SIZE; i++)
        {
            counter[i]++;
        }
        for (i = 0; i < CTX_SIZE; i++)
        {
            securityLevel = calcSecurityLevel(md[i], IN1, counter[i]);
            if(maxSecurityLevel < securityLevel){
              printf("Max update: %" PRId64" -> %" PRId64"\n", maxSecurityLevel, securityLevel);
              maxSecurityLevel = securityLevel;
            }
            if(securityLevel >= 32){
              printf("verifier : %24" PRIu64", ", counter[i]);
              for (j = 0; j < SHA_DIGEST_LENGTH; j++)
              {
                  printf("%02x", md[i][j]);
              }
              printf("\n");
            }
        }
    }
out:
    return 0;
}
