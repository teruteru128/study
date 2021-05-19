
#include "config.h"
#include "gettext.h"
#include <stddef.h>
#define _(str) gettext(str)
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

extern char *ltoa(long, char *, int);

void showNabeatsu(size_t nmax)
{
    size_t tmp = 0;
    size_t flg;
    for (size_t n = 1; n <= nmax; n++)
    {
        flg = 0;
        tmp = n;
        while(tmp > 0)
        {
            flg = flg | ((tmp % 10UL) == 3UL);
            tmp /= 10;
        }
        if (flg != 0 || n % 3 == 0)
        {
            fputs("aho\n", stdout);
        }
        else
        {
            fprintf(stdout, "%zu\n", n);
        }
    }
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    showNabeatsu(40);
}
