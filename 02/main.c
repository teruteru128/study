
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

int main(int argc, char **argv)
{
    SHA_CTX *c;
    unsigned char md[SHA_DIGEST_LENGTH];
    int i;
    char* in1 = "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s=";
    char* in2 = "372215731732267126";

    c = malloc(sizeof(SHA_CTX));
    if (c == NULL)
    {
        exit(EXIT_FAILURE);
    }
    memset(c, 0, sizeof(SHA_CTX));
    SHA1_Init(c);
    SHA1_Update(c, in1, strlen(in1));
    SHA_Update(c, in2, strlen(in2));
    SHA1_Final(md, c);

    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        printf("%02x", md[i]);
    }
    printf("\n");
    free(c);
    return EXIT_SUCCESS;
}
