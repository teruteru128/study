
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <features.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    size_t tmp = 4;
    if (argc >= 2)
    {
        errno = 0;
        tmp = strtoul(argv[1], NULL, 10);
        if (errno != 0)
        {
            tmp = 4;
        }
    }
    size_t padding_size = tmp;

    char buf[BUFSIZ];
    char *work;
    size_t i = 0;
    for (i = 0; i < padding_size; i++)
    {
        fputc('\n', stdout);
    }
    while (fgets(buf, BUFSIZ, stdin) != NULL)
    {
        work = strpbrk(buf, "\r\n");
        if (work != NULL)
        {
            *work = '\0';
        }
        for (i = 0; i < padding_size; i++)
        {
            fputc(' ', stdout);
        }
        printf("%s\n", buf);
    }
    return 0;
}
