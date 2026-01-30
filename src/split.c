
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <gmp.h>
#include <endian.h>

#define E 16777216
#define ES (E * 64)

int main(int argc, char *argv[], char *envp[])
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <sieve file> <number file> [max-prime]\n", argv[0]);
        return 1;
    }
    FILE *sievef = fopen(argv[1], "rb");
    FILE *numberf = fopen(argv[2], "r");
    uint64_t maxprime = argc >= 4 ? strtoull(argv[3], NULL, 10) : 1048576 * 2 + 1;
    if(sievef == NULL || numberf == NULL)
    {
        perror("fopen");
        if(sievef != NULL){
            fclose(sievef);
        }
        if(numberf != NULL){
            fclose(numberf);
        }
        return 1;
    }
    fseek(sievef, 8, SEEK_SET);
    uint64_t req_sieve_length = ((maxprime - 1) / 128) + 1;
    uint64_t *sieve = malloc(sizeof(uint64_t) * req_sieve_length);
    ssize_t sievelength = fread(sieve, sizeof(uint64_t), req_sieve_length, sievef);
    if(sievelength != req_sieve_length)
    {
        perror("fread");
        fclose(sievef);
        fclose(numberf);
        return 1;
    }
    fclose(sievef);
    uint64_t i;
    for(i = 0; i < sievelength; i++)
    {
        sieve[i] = be64toh(sieve[i]);
    }
    char buffer[BUFSIZ];
    char * ret = fgets(buffer, BUFSIZ, numberf);
    if(ret == NULL)
    {
        perror("fgets");
        return 1;
    }
    mpz_t n;
    mpz_init_set_str(n, buffer, 10);

    uint64_t r;
    uint64_t maxindex = req_sieve_length * 64;
    for(i = 1; i < maxindex; i++)
    {
        if(((sieve[i >> 6] >> (i & 0x3f)) & 1) == 0)
        {
            // prime
            r = mpz_fdiv_ui(n, i * 2 + 1);
            if(r == 0)
            {
                printf("%" PRIu64 "\n", i*2+1);
            }
        }
    }
    mpz_clear(n);
    free(sieve);
    return 0;
}
