
#include "config.h"
#include "textdomain_inner.h"
#include "mytextdomain.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>

void inittextdomain(void)
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
}
