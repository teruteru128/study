
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>
#include <uuid/uuid.h>

static void generate_output_filename(char *dest, size_t maxlen, size_t bit_length)
{
    if (dest == NULL)
        return;

    uuid_t uuid;
    char uuid_str[UUID_STR_LEN];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);

    snprintf(dest, maxlen, "%lubit-%s-initialValue.txt", bit_length, uuid_str);
}

/**
 * @brief 素数探索の初期値を生成
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    const size_t bit_length = 1024;
    const size_t buffer_size = ((bit_length - 1) >> 3) + 1;
    mpz_t base;
    mpz_inits(base, NULL);
    unsigned char *p = calloc(buffer_size, sizeof(char));
    FILE *fin = fopen("/dev/urandom", "rb");
    fread(p, sizeof(unsigned char), buffer_size, fin);
    fclose(fin);
    mpz_import(base, buffer_size, 1, sizeof(char), 0, 0, p);
    free(p);
    p = NULL;

    mpz_setbit(base, bit_length - 1);
    mpz_clrbit(base, 0);

    char dest[PATH_MAX];
    generate_output_filename(dest, PATH_MAX, bit_length);

    FILE *fout = fopen(dest, "w");
    if (fout == NULL)
    {
        perror("Failed to open the output file.");
        mpz_clear(base);
        return EXIT_FAILURE;
    }
    mpz_out_str(fout, 16, base);
    fputs("\n", fout);
    fclose(fout);
    mpz_clear(base);
    return EXIT_SUCCESS;
}
