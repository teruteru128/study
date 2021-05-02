
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <errno.h>

#define SIZE 16777216

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *threadFunc(void *arg)
{
    uint64_t table[SIZE];

    for (long i = 0; i < SIZE; i++)
    {
        table[i] = (uint64_t)((double)i * 3.14);
    }
    fprintf(stderr, "table size is %zu\n", sizeof(table));
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    fprintf(stderr, "準備完了\n");
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    for (size_t i = 0; i < SIZE; i++)
    {
        fprintf(stdout, "%lu\n", table[i]);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    size_t stacksize;
    pthread_attr_getstacksize(&attr, &stacksize);
    fprintf(stderr, "%lu -> ", stacksize);

    stacksize += SIZE * sizeof(uint64_t);
    fprintf(stderr, "%lu\n", stacksize);

    if (pthread_attr_setstacksize(&attr, stacksize) != 0)
    {
        perror("Failed to set stack size.");
        return EXIT_FAILURE;
    }

    pthread_t thread;
    if (pthread_create(&thread, &attr, threadFunc, NULL) != 0)
    {
        perror("Error: Failed to create new thread.");
        return EXIT_FAILURE;
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);

    fprintf(stderr, "エンターキーを押して再開...\n");
    pthread_mutex_lock(&mutex);
    fgetc(stdin);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    if (pthread_join(thread, NULL) != 0)
    {
        perror("Error: Failed to wait for the thread termination.");
        return EXIT_FAILURE;
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_attr_destroy(&attr);
    return EXIT_SUCCESS;
}
