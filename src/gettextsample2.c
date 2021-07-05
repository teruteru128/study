
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gettextsample2.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <stdio.h>
#include <locale.h>

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    printf(_("Hello world!\n"));
    return 0;
}
