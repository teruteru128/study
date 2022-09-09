
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

void *threadFunc(void *arg)
{
    struct timespec ts;
    printf("threadFunc: Start\n");
    sleep(2);
    pthread_mutex_lock(&mutex);
    printf("threadFunc: Wait for signal\n");
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;
    switch (pthread_cond_timedwait(&cond, &mutex, &ts))
    {
    case 0:
        fprintf(stderr, "threadFunc: Got signal\n");
        break;
    case ETIMEDOUT:
        fprintf(stderr, "threadFunc: Timeout\n");
        break;
    default:
        fprintf(stderr, "threadFunc: Error on pthread_cond_wait\n");
        exit(1);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(void)
{
    pthread_t thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread, NULL, threadFunc, NULL);
    sleep(1);
    fprintf(stderr, "main: Signal\n");
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
