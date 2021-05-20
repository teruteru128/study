
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <gmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    mpz_t hex;
    mpz_init(hex);

    int c;
    int con = 1;
    while (con)
    {
        while (1)
        {
            c = fgetc(stdin);
            if (isdigit(c) && '1' <= c && c <= '6')
            {
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
        gmp_fprintf(stdout, "(%zubit)%Zx\n", mpz_sizeinbase(hex, 2), hex);
    }

    mpz_clear(hex);
    return 0;
}
