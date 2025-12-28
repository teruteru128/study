
#include "ochinchin.h"
#include "eja.h"
#include "ftnr.h"
#include "penis.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    size_t length = 27;
    for (size_t i = 1; length <= 300; i++)
    {
        printf("第%ld形態: %zucm\n", i, length);
        penis(length);
        length += i * 3;
    }
    ftnr_penis(argc, argv);
    return EXIT_SUCCESS;
}
