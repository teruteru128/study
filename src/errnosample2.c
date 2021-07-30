
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)
#include <errno.h>
#include <locale.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
    int ret;
#else
    char *ret;
#endif
    char buf[BUFSIZ] = "";
    for (int i = 0; i < 134; i++)
    {
        buf[0] = 0;
        errno = 0;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
        ret = strerror_r(i, buf, BUFSIZ);
        if (ret == 0)
            printf("! %3d %s\n", i, buf);
        else
            printf("? %3d %d, %s\n", i, ret, buf);
#else
        ret = strerror_r(i, buf, BUFSIZ);
        if (ret == buf)
            printf("! %3d %s\n", i, buf);
        else
            printf("? %3d %s, %s\n", i, ret, buf);
#endif
    }
    return EXIT_SUCCESS;
}
