
/*
    https://emotionexplorer.blog.fc2.com/blog-entry-111.html
*/

#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define LOOPNUM 4

void funcA(const int n)
{
    for (int i = 0;i < LOOPNUM;i++) {
        printf("funcA t=%d %d\n", omp_get_thread_num(), n);
        sleep(1);
    }
}
void funcB(const int n)
{
    for (int i = 0;i < LOOPNUM;i++) {
        printf("funcB t=%d %d\n", omp_get_thread_num(), n);
        sleep(1);
    }
}
void funcC(const int n)
{
    for (int i = 0;i < LOOPNUM;i++) {
        printf("funcC t=%d %d\n", omp_get_thread_num(), n);
        sleep(1);
    }
}

int main ()
{
    printf("スレッド情報 omp_get_max_threads=%d\n", omp_get_max_threads());
    printf("スレッド情報 omp_get_num_procs=%d\n", omp_get_num_procs());
    printf("スレッド情報 omp_get_num_threads=%d\n", omp_get_num_threads());
    printf("スレッド情報 omp_get_thread_num=%d\n", omp_get_thread_num());

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            funcA(1);
        }
        #pragma omp section
        {
            funcB(2);
        }
        #pragma omp section
        {
            funcC(3);
        }
    }
    printf("終了\n");

    return 0;
}
