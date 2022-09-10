
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "bitsieve.h"
#include <gmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 拡張子書き換え
 *
 * @param out
 * @param outlen
 * @param in
 * @return int
 */
int replaceextension(char *out, size_t outlen, char *in, char *ext)
{
    if (out == NULL || in == NULL || ext == NULL)
    {
        return 1;
    }
    char *work = strdup(in);
    char *dot = strrchr(work, '.');
    if (dot != NULL)
    {
        *dot = '\0';
    }
    snprintf(out, outlen, "%s.%s", work, ext);
    free(work);
    return 0;
}

// bitsieveをエクスポートして毎回使い回せば早くなるんじゃねえか？作戦
int exportBitSieve_main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);

    {
        FILE *fin = fopen(argv[1], "r");
        mpz_inp_str(base, fin, 16);
        fclose(fin);
        fin = NULL;
    }

    const size_t searchLength = mpz_sizeinbase(base, 2) / 20 * 64;
    // printf("%lu\n", searchLength);
    struct BitSieve *bitSieve = bs_new();
    bs_initInstance(bitSieve, &base, searchLength);

    char outfilename[FILENAME_MAX] = "";
    replaceextension(outfilename, FILENAME_MAX, argv[1], "bs");
    {
        FILE *fout = fopen(outfilename, "wb");
        bs_fileout(fout, bitSieve);
        fclose(fout);
    }
    bs_free(bitSieve);
    mpz_clear(base);
    return 0;
}

/**
 * @brief ビット篩を生成してファイルに書き出し
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, const char *argv[])
{
    return exportBitSieve_main(argc, argv);
}
