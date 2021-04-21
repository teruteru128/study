
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

void *func(void *arg)
{
    return __errno_location();
}

// 上限は32753
#define THREAD_NUM (1 << 14)

int main(int argc, char const *argv[])
{
    int *e = __errno_location();
    printf("%lu, %p, %d <- main\n", (unsigned long)e, (void *)e, *e);
    pthread_t *threads = malloc(THREAD_NUM * sizeof(pthread_t));

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(threads + i, NULL, func, NULL);
    }

    int *errnos;

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], (void *)&errnos);
        printf("%lu, %p, %d\n", (unsigned long)errnos, (void *)errnos, *errnos);
    }
    free(threads);

    return 0;
}
