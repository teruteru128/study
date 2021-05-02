
#include "config.h"
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
#include <inttypes.h>

extern char *ltoa(long, char *, int);

void showNabeatsu(size_t nmax)
{
#ifdef ENABLE_REGEX
    regex_t pattern1;
    regcomp(&pattern1, "3", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
#endif
    char txt[24];
    for (size_t n = 1; n <= nmax; n++)
    {
        snprintf(txt, 24, "%zu", n);
        //ltoa(n, txt, 10);
        if (
#ifdef ENABLE_REGEX
                regexec(&pattern1, buf, 0, NULL, 0) == 0
#else
                strchr(txt, '3')
#endif
                || n % 3 == 0)
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
#ifdef ENABLE_REGEX
    regfree(&pattern1);
#endif
}
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    showNabeatsu(INT_MAX);
}
