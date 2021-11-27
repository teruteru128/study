#include <omp.h>
#include <stdio.h>
#include <stdatomic.h>

int main()
{
    atomic_int cnt = 0;
#pragma omp parallel
    {
        printf("%d\n", cnt++);
    }
    return 0;
}
