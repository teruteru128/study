
#define _GNU_SOURCE
#include "config.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define STR "やったぜ。"

int mbsample(void)
{
    setlocale(LC_CTYPE, "ja_JP.UTF-8");
    char string[] = STR;
    mbstate_t ps = { 0 };
    int length = mbrlen(string, MB_CUR_MAX, &ps);
    if (length < 0)
    {
        perror("mbrlen");
        return EXIT_FAILURE;
    }
    wchar_t array[6];
    int tmp = mbtowc(array, string, length);
    array[1] = L'\0';
    printf("wide character string: %ls\n", array);
    return 0;
}

int hiho(int argc, char **argv)
{
    if (argc > 1)
        printf("%c\n", (argv[1][0] + 8) & 0x7f);
    else
        printf("%c\n", 'G' + 8);
    return 0;
}
