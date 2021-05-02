
#include <string.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gmp.h>

/**
 * @brief
 * ファイルから読みだした初期値とオフセットを加算してファイルに書き出します。
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(const int argc, const char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "%s <infilename> <offset>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *infilename = argv[1];
    const unsigned long offset = strtoul(argv[2], NULL, 10);
    size_t innamelen = strlen(infilename);
    char *outfilename = malloc(innamelen + 32 + 1);
    memcpy(outfilename, infilename, innamelen + 1);
    char *outtmp = NULL;
    char *retmp = NULL;
    size_t outnamelen = 0;
    if ((outtmp = strrchr(outfilename, '-')) != NULL)
    {
        *outtmp = '\0';
    }
    else
    {
        // ファイル名末尾のヌルバイトを入れる
        outtmp = outfilename + innamelen;
    }
    strncat(outtmp, "-prime.txt", 32);
    outnamelen = strlen(outfilename) + 1;
    retmp = realloc(outfilename, outnamelen);
    if (retmp != NULL)
    {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    outfilename = retmp;

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
