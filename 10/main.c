
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>

int main(int argc, char **argv)
{
    char *str = getenv("PATH");
    printf("%p\n", str);
    printf("%s\n", str);
    str = getenv("LANG");
    printf("%p\n", str);
    printf("%s\n", str);
    return 0;
}
