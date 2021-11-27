#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    size_t i;
#pragma omp parallel for
    for (i = 0; i < 1024; ++i)
    {
        // ここが並列に処理されます。並列数は、コア数分です。
        sleep(1);
        printf("hello world: %lu\n", i);
    }
    return EXIT_SUCCESS;
}
