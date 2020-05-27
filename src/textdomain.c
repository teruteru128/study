
#include "textdomain_inner.h"
#include "mytextdomain.h"
#include <libintl.h>

void inittextdomain(void) {
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
}
