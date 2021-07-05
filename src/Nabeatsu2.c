
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#define _(str) gettext(str)
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern char *ltoa(long, char *, int);

void showNabeatsu(size_t nmax)
{
    regex_t pattern1;
    regcomp(&pattern1, "3", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
    char txt[24];
    for (size_t n = 1; n <= nmax; n++)
    {
        snprintf(txt, 24, "%zu", n);
        // ltoa(n, txt, 10);
        if (regexec(&pattern1, txt, 0, NULL, 0) == 0 || n % 3 == 0)
        {
            fputs("aho\n", stdout);
        }
        else
        {
            // fprintf(stdout, "%s\n", txt);
            fputs(txt, stdout);
            fputs("\n", stdout);
        }
    }
    regfree(&pattern1);
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    showNabeatsu(INT_MAX);
}
