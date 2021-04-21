
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <byteswap.h>
#include <string.h>
#define PUBLISH_STRUCT_BS
#include "bitsieve.h"

// bitsieveをエクスポートして毎回使い回せば早くなるんじゃねえか？作戦
int exportBitSieve_main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);

    FILE *fin = fopen(argv[1], "r");
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    fin = NULL;

    const size_t searchLength = mpz_sizeinbase(base, 2) / 20 * 64;
    //printf("%lu\n", searchLength);
    struct BitSieve bitSieve;
    bs_initInstance(&bitSieve, &base, searchLength);

    char outfilename[FILENAME_MAX] = "";
#if 1
    {
        // 拡張子書き換え
        char *work = strdup(argv[1]);
        char *dot = strrchr(work, '.');
        if (dot != NULL)
        {
            *dot = '\0';
        }
        snprintf(outfilename, FILENAME_MAX, "%s.bs", work);
        free(work);
    }
#else
    // 拡張子追加して置き換え
    snprintf(outfilename, FILENAME_MAX, "%s.bs", argv[1]);
#endif
    FILE *fout = fopen(outfilename, "wb");
    bs_fileout(fout, &bitSieve);
    fclose(fout);
    bs_free(&bitSieve);
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
