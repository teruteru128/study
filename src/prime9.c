
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <gmp.h>

/**
 * @brief ファイルから読みだした初期値とオフセットを加算してファイルに書き出します。
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(const int argc, const char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "%s <outfilename> <infilename> <offset>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *outfilename = argv[1];
    const char *infilename = argv[2];
    const unsigned long offset = strtoul(argv[3], NULL, 10);

    if (access(infilename, F_OK | R_OK) == 0)
    {
        fprintf(stderr, "%s is unreadable.\n", infilename);
        return EXIT_FAILURE;
    }

    FILE *fin = fopen(infilename, "r");
    FILE *fout = fopen(outfilename, "w");
    if (fin == NULL || fout == NULL)
    {
        if (fin != NULL)
            fclose(fin);
        if (fout != NULL)
            fclose(fout);
        perror("fin");
        return EXIT_FAILURE;
    }
    mpz_t base;
    mpz_init(base);
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    mpz_add_ui(base, base, offset);
    mpz_out_str(fout, 16, base);
    fclose(fout);

    return EXIT_SUCCESS;
}
