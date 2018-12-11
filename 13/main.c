
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>

int main(int argc, char **argv)
{
    locale_t loc = NULL, nloc;
    loc = uselocale(NULL);
    if (loc == NULL)
    {
        perror(NULL);
        return EXIT_FAILURE;
    }
    nloc = duplocale(loc);
    if (nloc == NULL)
    {
        perror(NULL);
        return EXIT_FAILURE;
    }
    freelocale(loc);
    freelocale(nloc);
    nloc = NULL;
    return 0;
}
