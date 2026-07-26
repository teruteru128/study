
#include <time.h>
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
 * @brief large-sieveを生成してファイルに書き出し
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "%s <input even number file> <output large sieve file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);

    char *base_filename = argv[1];
    {
        FILE *fin = fopen(base_filename, "r");
        mpz_inp_str(base, fin, 16);
        fclose(fin);
        fin = NULL;
    }

    const size_t searchLength = (size_t)(mpz_sizeinbase(base, 2) / 20.0 * 64);
    // printf("%lu\n", searchLength);
    struct BitSieve *largeSieve = bs_new();
    struct timespec startt;
    struct tm tm;
    clock_gettime(CLOCK_REALTIME, &startt);
    localtime_r(&startt.tv_sec, &tm);
    char time_buffer[64];
    strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    printf("%s: 基準偶数に対するlarge-sieveの生成を開始します...\n", time_buffer);
    clock_gettime(CLOCK_MONOTONIC, &startt);
    bs_initInstance(largeSieve, &base, searchLength);
    struct timespec finish;
    clock_gettime(CLOCK_MONOTONIC, &finish);
    struct timespec diff;
    difftimespec(&diff, &finish, &startt);
    clock_gettime(CLOCK_REALTIME, &finish);
    localtime_r(&finish.tv_sec, &tm);
    strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    printf("%s: 篩の初期化を完了しました. (%ld.%09lds)\n", time_buffer, diff.tv_sec, diff.tv_nsec);

    char *outfilename = argv[2];
    {
        FILE *fout = fopen(outfilename, "wb");
        bs_fileout(fout, largeSieve);
        fclose(fout);
    }
    bs_free(largeSieve);
    mpz_clear(base);
    return 0;
}
