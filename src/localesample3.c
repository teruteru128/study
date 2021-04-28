
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
    locale_t loc = uselocale((locale_t)0);
    char *msg = strerror_l(0, loc);
    printf("%s\n", msg);
    return 0;
}
