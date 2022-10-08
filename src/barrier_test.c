/* https://codezine.jp/article/detail/1973?p=2
 gcc barrier_test.c -o barrier_test -W -Wall -g -lpthread */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;

#define THREAD_MAX 8
#define LOOP_CNT 10000000
#define TV2SEC(tv) ((double)((tv).tv_sec) + (double)((tv).tv_usec / 1000000.0))
void *thread_func(void *arg);

int main()
{

    pthread_t id[THREAD_MAX];
    double *result[THREAD_MAX];
    int i;

    pthread_barrier_init(&barrier, 0, THREAD_MAX);

    pthread_mutex_lock(&mutex);
    for (i = 0; i < THREAD_MAX; i++)
    {
        pthread_create(&id[i], 0, thread_func, (void *)&i);
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    for (i = 0; i < THREAD_MAX; i++)
    {
        pthread_join(id[i], (void **)&result[i]);
    }
    for (i = 0; i < THREAD_MAX; i++)
    {
        printf("thread %d end ... timesec:[%f]\n", i, *result[i]);
    }
    return 0;
}

void *thread_func(void *arg)
{

    pthread_mutex_lock(&mutex);
    int index = *(int *)arg;
    double *rtn = malloc(sizeof(double));
    struct timeval tv1, tv2;
    int count, i;

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    gettimeofday(&tv1, 0);
    for (i = 0, count = 0; i < LOOP_CNT; i++)
    {
        count++;
        if (count % 1000 == 0)
        {
            printf("thread %d \t\t%*.*s%4d\r", index, index + 1, index + 1,
                   "\t\t\t\t\t\t\t\t", count / 1000);
            fflush(stdout);
        }
        if (count % 1000000 == 0)
        {
            printf("thread %d Barrier!!\n", index);
            pthread_barrier_wait(&barrier);
            sleep(1);
            printf("thread %d Barrier release!!\n", index);
            pthread_barrier_wait(&barrier);
        }
    }
    gettimeofday(&tv2, 0);
    *rtn = TV2SEC(tv2) - TV2SEC(tv1);
    return (void *)rtn;
}
