
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>

/**
 * locale copy test
 * */
int main(int argc, char **argv)
{
    locale_t loc = uselocale((locale_t)0);
    if (loc == NULL)
    {
        perror("uselocale");
        return EXIT_FAILURE;
    }
    locale_t nloc = duplocale(loc);
    if (nloc == NULL)
    {
        perror("duplocale");
        return EXIT_FAILURE;
    }
    freelocale(nloc);
    return EXIT_SUCCESS;
}
