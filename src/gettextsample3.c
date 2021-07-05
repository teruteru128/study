
#include "mytextdomain.h"
#include "gettextsample3.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    useconds_t microseconds = 3.75 * 1000000;
    inittextdomain();

    printf(_("%dmicro seconds stopping.\n"), microseconds);
    usleep(microseconds);
    printf(_("%dmicro seconds stoped.\n"), microseconds);

    return EXIT_SUCCESS;
}
