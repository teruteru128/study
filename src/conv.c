
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    MP_INT num;
    mpz_init_set_str(&num, strncmp(argv[1], "0x", 2) == 0 ? argv[1] + 2 : argv[1], 16);
    size_t length = mpz_sizeinbase(&num, 10) + 2;
    char *str = malloc(length);
    mpz_get_str(str, 10, &num);
    printf("%s\n", str);
    free(str);
    mpz_clear(&num);
    return 0;
}
