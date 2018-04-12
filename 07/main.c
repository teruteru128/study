
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <signal.h>
#include <unistd.h>

#include <printint.h>
#define IN2_SIZE 20

static uint64_t *verifire_ptr = NULL;

static void act(int sig)
{
    fprintf(stderr, "verifier=%" PRIu64 "\n", *verifire_ptr);
}

int main(int argc, char **argv)
{
    SHA_CTX *c;
    struct timeval tv;
    unsigned char md[SHA_DIGEST_LENGTH];
    int i;
    char *in1 = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    char in2[IN2_SIZE];
    uint64_t in1Length = strlen(in1);
    uint64_t verifier;
    size_t verifierLength;

    verifire_ptr = &verifier;

    gettimeofday(&tv, NULL);
    srand48(((tv.tv_usec & 0xFFFFFFFF) << 32) ^ ((tv.tv_usec >> 32) & 0xFFFFFFFF) ^ tv.tv_sec);
    verifier = ((uint64_t)mrand48() << 33) ^ (((uint64_t)mrand48() << 16) & 0xffffffffULL) ^ (((uint64_t)mrand48()) & 0xffffffffULL);

    c = malloc(sizeof(SHA_CTX));
    if (c == NULL)
    {
        exit(EXIT_FAILURE);
    }
    memset(c, 0, sizeof(SHA_CTX));

    if (signal(SIGINT, act) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }

    printf("%" PRIu64 "d\n", getpid());
    for (;; verifier++)
    {
        SHA1_Init(c);
        SHA1_Update(c, in1, in1Length);
        verifierLength = snprintUInt64(in2, IN2_SIZE, verifier);
        SHA_Update(c, in2, verifierLength);
        SHA1_Final(md, c);
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
    return 0;
}
