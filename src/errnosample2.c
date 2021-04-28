
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    char *ret;
    char buf[BUFSIZ] = "";
    for (int i = 0; i < 134; i++)
    {
        buf[0] = 0;
        errno = 0;
        ret = strerror_r(i, buf, BUFSIZ);
        if (ret == buf)
            printf("! %3d %s\n", i, buf);
        else
            printf("? %3d %s, %s\n", i, ret, buf);
    }
    return 0;
}
