
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    const char *dir = getenv("TEXTDOMAINDIR");
    if (dir == NULL)
    {
        dir = LOCALEDIR;
    }
    printf("%s\n", dir);
    bindtextdomain(PACKAGE, dir);
    textdomain(PACKAGE);
    printf(_("Help me!\n"));
    printf(_("Hello world!\n"));
    return 0;
}
