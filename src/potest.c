
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    printf(_("Help me!\n"));
    return 0;
}
