
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *func(void *arg)
{
    pthread_mutex_t **mutexes = (pthread_mutex_t **)arg;
    pthread_mutex_lock(mutexes[1]);
    pthread_mutex_lock(mutexes[0]);
    pthread_mutex_unlock(mutexes[1]);
    pthread_mutex_unlock(mutexes[0]);
    return NULL;
}

int main(int argc, char const *argv[])
{
    pthread_t thread1;
    pthread_t thread2;
    pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *mutexes[2] = { NULL };

    mutexes[0] = &mutex1;
    mutexes[1] = &mutex2;
    pthread_create(&thread1, NULL, func, mutexes);
    mutexes[0] = &mutex2;
    mutexes[1] = &mutex1;
    pthread_create(&thread2, NULL, func, mutexes);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}
