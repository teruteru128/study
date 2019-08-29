
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <printint.h>

#define OUT_SIZE 21

int main(int argc, char **argv)
{
    uint64_t in = 1;
    char out1[OUT_SIZE];
    size_t i, length;
    memset(out1, 0, OUT_SIZE);

    for (i = 1; i <= 20; i++, in *= 10)
    {
        length = snprintUInt64(out1, OUT_SIZE, in);
        if (strncmp(out1, "10000000000000000000", length) == 0)
        {
            printf("OK\n");
        }
        else
        {
            printf("NG\n");
        }
    }
}
