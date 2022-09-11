
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "bitsieve.h"
#include "timeutil.h"
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
    printf("篩の初期化を開始します...\n");
    struct timespec startt;
    struct tm tm;
    clock_gettime(CLOCK_REALTIME, &startt);
    localtime_r(&startt.tv_sec, &tm);
    printf("%d/%d/%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
           tm.tm_hour, tm.tm_min, tm.tm_sec);
    clock_gettime(CLOCK_MONOTONIC, &startt);
    bs_initInstance(bitSieve, &base, searchLength);
    struct timespec finish;
    clock_gettime(CLOCK_MONOTONIC, &finish);
    struct timespec diff;
    difftimespec(&diff, &finish, &startt);
    clock_gettime(CLOCK_REALTIME, &finish);
    localtime_r(&finish.tv_sec, &tm);
    printf("%d/%d/%d %d:%d:%d: 篩の初期化を完了しました. (%ld.%09lds)\n",
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
           tm.tm_sec, diff.tv_sec, diff.tv_nsec);

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
