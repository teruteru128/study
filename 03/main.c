
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define LOOP_COUNT 100000000ULL

void printHelp(char *name)
{
    printf("%s [init|cpy]\n", name);
}

int main(int argc, char **argv)
{
    SHA_CTX *c0;
    SHA_CTX *c1;
    char *in1 = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    size_t in1Length = strlen(in1);
    uint64_t i;
    if (argc < 2)
    {
        printHelp(argv[0]);
        return 0;
    }
    c0 = malloc(sizeof(SHA_CTX));
    if (c0 == NULL)
    {
        exit(EXIT_FAILURE);
    }
    c1 = malloc(sizeof(SHA_CTX));
    if (c1 == NULL)
    {
        goto done;
    }
    if (strcmp(argv[1], "init"))
    {
        for (i = 0; i < LOOP_COUNT; i++)
        {
            SHA1_Init(c0);
            SHA1_Update(c0, in1, in1Length);
        }
    }
    else if (strcmp(argv[1], "cpy"))
    {
        SHA1_Init(c0);
        SHA1_Update(c0, in1, in1Length);
        for (i = 0; i < LOOP_COUNT; i++)
        {
            memcpy(c1, c0, sizeof(SHA_CTX));
        }
    }
    else
    {
        printHelp(argv[0]);
    }
done:
    free(c0);
    free(c1);
    return 0;
}
