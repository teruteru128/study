
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <alloca.h>
#include <err.h>
#include <locale.h>
#include <random.h>
#include <shuffle.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <wchar.h>

#define MAX 1000000

static size_t countmbchar(const char *work)
{
    size_t len = 0;
    size_t charnum = 0;
    mbstate_t st = { 0 };
    while (*work != '\0')
    {
        len = mbrlen(work, 30, &st);
        if (len == (size_t)-1)
        {
            perror("mbrlen");
            return 1;
        }
        work += len;
        charnum++;
    }
    return charnum;
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    ssize_t r = getrandom(getseedp(), sizeof(uint32_t), GRND_NONBLOCK);
    if (r < 0)
    {
        return 1;
    }
    size_t count = argc < 2 ? 1 : strtoul(argv[1], NULL, 10);

    char messages[] = "ファルコン・パンチ";
    size_t charnum = countmbchar(messages);
    size_t wstrlen = (charnum + 1) * sizeof(wchar_t);
    wchar_t *s = alloca(wstrlen);
    memset(s, 0, wstrlen);
    char *work = messages;
    mbstate_t st = { 0 };
    mbsrtowcs(s, (const char **)&work, charnum + 1, &st);
    for (size_t i = 0; i < count; i++)
    {
        shuffle(s, sizeof(wchar_t), wcslen(s));
        fprintf(stdout, "%ls\n", s);
    }

    return EXIT_SUCCESS;
}
