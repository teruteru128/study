
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <printint.h>

void showFizzBuzz(size_t a)
{
    int n;
    int tmp;
    for (n = 1; n <= a; n++)
    {
        if (n % 15 == 0)
        {
            printf("Fizz Buzz\n");
        }
        else if (n % 3 == 0)
        {
            printf("Fizz\n");
        }
        else if (n % 5 == 0)
        {
            printf("Buzz\n");
        }
        else
        {
            printf("%d\n", n);
        }
    }
}

#define MAX (100000000UL)
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    showFizzBuzz(MAX);
}
