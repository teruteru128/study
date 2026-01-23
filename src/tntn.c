
#include "ochinchin.h"
#include "eja.h"
#include "ftnr.h"
#include "penis.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
    if(strcmp(argv[1], "ftnr") == 0)
    {
        if(argc < 3)
        {
            fprintf(stderr, "引数不足\n");
            return 1;
        }
        uint64_t a = strtoull(argv[2], NULL, 10);
        ftnr_penis((int)a);
        return 0;
    }
    else if(strcmp(argv[1], "eja") == 0)
    {
        eja();
        return 0;
    }
    else if(strcmp(argv[1], "penis") == 0)
    {
        if(argc < 3)
        {
            fprintf(stderr, "引数不足\n");
            return 1;
        }
        penis(strtoull(argv[2], NULL, 10));
        return 0;
    }
}
