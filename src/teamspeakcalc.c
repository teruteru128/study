
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/time.h>
#include <openssl/sha.h>

#include "printint.h"
#define IN2_SIZE 21

int main(int argc, char **argv)
{
    SHA_CTX c;
    struct timeval tv;
    typedef union mc {
        unsigned char md[SHA_DIGEST_LENGTH];
        uint32_t ddd[SHA_DIGEST_LENGTH / sizeof(uint32_t)];
    } MC;
    MC mc;
    int i;
    char *in1 = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    char in2[IN2_SIZE];
    uint64_t in1Length = strlen(in1);
    uint64_t verifier;
    size_t verifierLength;

    gettimeofday(&tv, NULL);
    srand48(((tv.tv_usec & 0xFFFFFFFF) << 32) ^ ((tv.tv_usec >> 32) & 0xFFFFFFFF) ^ tv.tv_sec);
    /*
        1111111111111111111111111111111100000000000000000000000000000000
        0000000000000000111111111111111111111111111111110000000000000000
        0000000000000000000000000000000011111111111111111111111111111111
    */
    verifier = mrand48() & 0xffffffffULL;
    verifier = (verifier << 16) ^ (mrand48() & 0xffffffffULL);
    verifier = (verifier << 16) ^ (mrand48() & 0xffffffffULL);
    memset(&c, 0, sizeof(SHA_CTX));
    for (;; verifier++)
    {
        SHA1_Init(&c);
        SHA1_Update(&c, in1, in1Length);
        verifierLength = snprintf(in2, IN2_SIZE, "%ld", verifier);
        SHA1_Update(&c, in2, verifierLength);
        SHA1_Final(mc.md, &c);
        if (mc.md[0] == 0 && mc.md[1] == 0 && mc.md[2] == 0 && mc.md[3] == 0)
        {
            printf(_("verifier : %" PRIu64 "\n"), verifier);
            for (i = 0; i < SHA_DIGEST_LENGTH; i++)
            {
                printf("%02x", mc.md[i]);
            }
            printf("\n");
            break;
        }
    }
    return EXIT_SUCCESS;
}
