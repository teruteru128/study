
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    printf("%p\n", (void *)__errno_location());
    printf("%d\n", *__errno_location());
    return 0;
}
