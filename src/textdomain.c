
#include "textdomain_inner.h"
#include <mytextdomain.h>
#include <gettext.h>

void inittextdomain(void) {
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
}
