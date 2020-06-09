
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>

/**
 * Environmental variable test
 * */
int main(int argc, char **argv)
{
    char *path = getenv("PATH");
    printf("%p\n", path);
    printf("%s\n", path);
    char *lang = getenv("LANG");
    printf("%p\n", lang);
    printf("%s\n", lang);
    ptrdiff_t a = path - lang;
    printf("%ld\n", a);
    return EXIT_SUCCESS;
}
