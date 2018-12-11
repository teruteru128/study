
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <locale.h>

int main(int argc, char **argv)
{
    char *locale = setlocale(LC_ALL, "");
    printf("%s\n", locale);
    return 0;
}
