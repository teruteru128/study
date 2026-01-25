
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
        fprintf(stderr, "Usage: %s <sieve file> <number file>\n", argv[0]);
        return 1;
    }
    FILE *sievef = fopen(argv[1], "rb");
    FILE *numberf = fopen(argv[2], "r");
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
    // 1,048,576 bit
    uint64_t *sieve = malloc(sizeof(uint64_t) * E);
    ssize_t sievelength = fread(sieve, sizeof(uint64_t), E, sievef);
    if(sievelength != E)
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
    for(i = 1; i < ES; i++)
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
