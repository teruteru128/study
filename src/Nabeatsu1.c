
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#ifdef ENABLE_REGEX
#include <regex.h>
#else
#include <string.h>
#endif
#include <limits.h>
#include <stdint.h>

extern char *ltoa(long, char *, int);

void showNabeatsu(size_t nmax)
{
    char txt[24];
    for (size_t n = 1; n <= nmax; n++)
    {
        snprintf(txt, 24, "%zu", n);
        //ltoa(n, txt, 10);
        if ( strchr(txt, '3') || n % 3 == 0)
        {
            fputs("aho\n", stdout);
        }
        else
        {
            //fprintf(stdout, "%s\n", txt);
            fputs(txt, stdout);
            fputs("\n", stdout);
        }
    }
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    showNabeatsu(INT_MAX);
}
