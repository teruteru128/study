
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include "localesample1.h"

int main(int argc, char **argv)
{
    char *locale = setlocale(LC_ALL, "");
    printf("%p : %s\n", locale, locale);
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    printf(_("Hello world!\n"));
    return 0;
}
