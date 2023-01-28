
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 128

static volatile int active = 1;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int waiting = 0;
static int ns[NUM_THREADS] = { 0 };

static void *func(void *arg)
{
    size_t idx = (size_t)arg;
    printf("thread[%zu] start\n", idx);
    while (active)
    {
        pthread_mutex_lock(&mtx);
        waiting++;
        pthread_cond_wait(&cv, &mtx);
        ns[idx]++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    pthread_t ths[NUM_THREADS];
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&ths[i], NULL, func, (void *)i);
    }

    for (size_t i = 0; i < 10000000; i++)
    {
        pthread_mutex_lock(&mtx);
        if (waiting > 0)
        {
            waiting--;
            pthread_cond_signal(&cv);
            if(i % 100000 == 99999)
            printf("woke! %zu\n", i);
        }
        else
        {
            i--;
        }
        pthread_mutex_unlock(&mtx);
    }
    size_t total = 0;
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        printf("n[%d]\n", ns[i]);
        total += ns[i];
    }
    printf("total = %zu\n", total);

    active = 0;
    pthread_mutex_lock(&mtx);
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(ths[i], NULL);
    }

    return 0;
}
