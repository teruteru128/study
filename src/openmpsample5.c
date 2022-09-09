#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    fprintf(stdout, "スレッド情報 omp_get_max_threads=%d\n", omp_get_max_threads());
    fprintf(stdout, "スレッド情報 omp_get_num_procs=%d\n", omp_get_num_procs());
    fprintf(stdout, "スレッド情報 omp_get_num_threads=%d\n", omp_get_num_threads());
    fprintf(stdout, "スレッド情報 omp_get_thread_num=%d\n", omp_get_thread_num());
    size_t i;
#pragma omp parallel for
    for (i = 0; i < 1024; ++i)
    {
        // ここが並列に処理されます。並列数は、コア数分です。
        fprintf(stdout, "hello world: %lu, %d\n", i, omp_get_thread_num());
    }
    return EXIT_SUCCESS;
}
