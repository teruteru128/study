
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>
#include <uuid/uuid.h>

#define BIT_LENGTH 524288
#define BUFFER_SIZE (((BIT_LENGTH - 1) >> 3) + 1)

static void generate_output_filename(char *filename, size_t maxlen)
{
    if (filename == NULL)
        return;

    uuid_t uuid;
    char uuid_str[UUID_STR_LEN];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);

    snprintf(filename, maxlen, "%dbit-%s-initialValue.txt", BIT_LENGTH, uuid_str);
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
    mpz_t base;
    mpz_inits(base, NULL);
    unsigned char p[BUFFER_SIZE];
    FILE *fin = fopen("/dev/urandom", "rb");
    fread(p, sizeof(unsigned char), BUFFER_SIZE, fin);
    fclose(fin);
    mpz_import(base, BUFFER_SIZE, 1, sizeof(char), 0, 0, p);

    mpz_setbit(base, BIT_LENGTH - 1);
    mpz_clrbit(base, 0);

    char filename[PATH_MAX];
    generate_output_filename(filename, PATH_MAX);

    FILE *fout = fopen(filename, "w");
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
