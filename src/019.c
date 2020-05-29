
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 65536

int main(int argc, char **argv)
{
    size_t i = 0;
    size_t j = 0;
    size_t sum = 0;
    for(i = 0; i < SIZE; i++)
    {
        sum++;
        for(j = 0; j < i; j++)
        {
            sum++;
            sum++;
        }
    }
    printf("%ld\n", sum);
    return EXIT_SUCCESS;
}
