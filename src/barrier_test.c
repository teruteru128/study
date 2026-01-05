/* https://codezine.jp/article/detail/1973?p=2
 gcc barrier_test.c -o barrier_test -W -Wall -g -lpthread */
#define _GNU_SOURCE 1
#define _DEFAULT_SOURCE
#define _POSIX_SOURCE
#define _BSD_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t init_barrier;
pthread_barrier_t barrier;

#define THREAD_MAX 8
#define LOOP_CNT 10000000
#define TV2SEC(tv) ((double)((tv).tv_sec) + (double)((tv).tv_usec / 1000000.0))
void *thread_func(void *arg);

struct arguments
{
    int index;
};

int main(int argc, char *argv[], char *envp[])
{

    pthread_t id[THREAD_MAX];
    double *result[THREAD_MAX];
    struct arguments args[THREAD_MAX];
    int i;

    pthread_barrier_init(&init_barrier, 0, THREAD_MAX + 1);
    pthread_barrier_init(&barrier, 0, THREAD_MAX);

    for (i = 0; i < THREAD_MAX; i++)
    {
        args[i].index = i;
        pthread_create(&id[i], 0, thread_func, (void *)(args + i));
    }
    pthread_barrier_wait(&init_barrier);
    for (i = 0; i < THREAD_MAX; i++)
    {
        pthread_join(id[i], (void **)(result + i));
    }
    for (i = 0; i < THREAD_MAX; i++)
    {
        printf("thread %d end ... timesec:[%f]\n", i, result[i][0]);
        free(result[i]);
    }
    pthread_barrier_destroy(&init_barrier);
    pthread_barrier_destroy(&barrier);
    return 0;
}

void *thread_func(void *arg)
{

    int index = ((struct arguments *)arg)->index;
    double *rtn = malloc(sizeof(double));
    struct timespec ts1, ts2;
    int count, i;

    pthread_barrier_wait(&init_barrier);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    for (i = 0, count = 0; i < LOOP_CNT; i++)
    {
        count++;
        if (count % 1000 == 0)
        {
            pthread_mutex_lock(&mutex);
            printf("thread %d \t\t\t\t\t\t\t\t\t\t%4d\n", index, count / 1000);
            fflush(stdout);
            pthread_mutex_unlock(&mutex);
        }
        if (count % 1000000 == 0)
        {
            pthread_mutex_lock(&mutex);
            printf("thread %d Barrier!!\n", index);
            pthread_mutex_unlock(&mutex);
            pthread_barrier_wait(&barrier);
            sleep(1);
            pthread_mutex_lock(&mutex);
            printf("thread %d Barrier release!!\n", index);
            pthread_mutex_unlock(&mutex);
            pthread_barrier_wait(&barrier);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    long diffnsec;
    long diff;
    if ((ts2.tv_nsec - ts1.tv_nsec) < 0)
    {
        diff = ts2.tv_sec - ts1.tv_sec - 1;
        diffnsec = ts2.tv_nsec - ts1.tv_nsec + 1000000000;
    }
    else
    {
        diff = ts2.tv_sec - ts1.tv_sec;
        diffnsec = ts2.tv_nsec - ts1.tv_nsec;
    }
    *rtn = diff + ((double)diffnsec) / 1e9;
    return (void *)rtn;
}
