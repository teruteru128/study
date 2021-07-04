
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <gmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    mpz_t hex;
    mpz_init(hex);

    size_t size = 256;
    char *dice = malloc(size);
    memset(dice, 0, size);
    size_t length = 0;

    int c;
    int con = 1;
    printf("'q'で終了します。\n");
    while (con)
    {
        while (1)
        {
            c = fgetc(stdin);
            if (isdigit(c) && '1' <= c && c <= '6')
            {
                dice[length++] = (char)c;
                if (length == size)
                {
                    char *tmp = realloc(dice, size * 2);
                    if (tmp != NULL)
                    {
                        dice = tmp;
                        size *= 2;
                        memset(dice + length, 0, size - length);
                    }
                    else
                    {
                        perror("realloc");
                        free(dice);
                        exit(EXIT_FAILURE);
                    }
                }
                mpz_mul_ui(hex, hex, 6);
                mpz_add_ui(hex, hex, (unsigned long)(c - '1'));
            }
            else if (c == 'c' || c == 'f' || c == 'q' || c == EOF)
            {
                if (ferror(stdin) != 0)
                {
                    perror("fgetc");
                }
                con = 0;
                break;
            }
            else if (c == '\n')
            {
                break;
            }
            else
            {
                fprintf(stderr, "invarit charactor!: (0x%02x)\n", c);
            }
        }
        gmp_fprintf(stdout, "(%zudigits)%s\n(%zubit)%Zx\n", length, dice,
                    mpz_sizeinbase(hex, 2), hex);
    }
    free(dice);
    mpz_clear(hex);
    return 0;
}
