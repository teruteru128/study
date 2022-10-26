
#include "eja.h"
#include "ftnr.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    size_t length = 27;
    for (size_t i = 1; length < 300; i++)
    {
        printf("第%ld形態: %zucm\n", i, length);
        length += i * 3;
    }
    return 0;
}
