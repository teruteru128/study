#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    size_t i = 0;
#pragma omp parallel for
    for (i = 0; i < 16; ++i)
    {
        printf("hello world: %d\n", omp_in_final());
    }
    printf("hello world: %d\n", omp_in_final());
    return EXIT_SUCCESS;
}
