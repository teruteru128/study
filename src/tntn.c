
#include "ochinchin.h"
#include "eja.h"
#include "ftnr.h"
#include "penis.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{
    /*
    size_t length = 27;
    for (size_t i = 1; length <= 300; i++)
    {
        printf("第%ld形態: %zucm\n", i, length);
        penis(length);
        length += i * 3;
    }
    */
    if (argc < 2)
    {
        return EXIT_SUCCESS;
    }
    uint64_t a = strtoull(argv[1], NULL, 10);
    ftnr_penis((int)a);
    return EXIT_SUCCESS;
}
