
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[])
{
    if(argc < 2)
    {
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if(!in)
    {
        perror("fopen");
        return 1;
    }
    unsigned char *buf = malloc(48 * 1024 * 1024);
    unsigned long length = fread(buf, sizeof(unsigned char), 48 * 1024 * 1024, in);
    printf("%zu\n", length);
    mpz_t n, power10;
    mpz_inits(n, power10, NULL);
    mpz_ui_pow_ui(power10, 10, 99999999);
    mpz_import(n, length, 1, sizeof(unsigned char), 0, 0, buf);
    free(buf);
    unsigned long length2 = mpz_sizeinbase(n, 10) + 2;
    char *str = malloc(length2);
    mpz_get_str(str, 10, n);
    printf("length: %zu\n", strlen(str));
    printf("n > power10: %d\n", mpz_cmp(n, power10) > 0);
    mpz_clears(n, power10, NULL);
    free(str);
    return 0;
}
