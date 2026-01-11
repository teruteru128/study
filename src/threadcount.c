
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <pthread.h>
#include <unistd.h>

struct count
{
    long count;
    long finish;
    pthread_mutex_t mutex;
};

void *funcyou(void *arg)
{
    struct count *a = (struct count *)arg;
    for (size_t i = 0; i < 0xffffffffL; i++)
    {
        pthread_mutex_lock(&a->mutex);
        a->count++;
        pthread_mutex_unlock(&a->mutex);
    }
    a->finish = 1;
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread;
    struct count a;
    a.count = 0;
    a.finish = 0;
    pthread_mutex_init(&a.mutex, NULL);

    if (pthread_create(&thread, NULL, funcyou, &a) != 0)
    {
        printf("Can't create thread\n");
        return EXIT_FAILURE;
    }

    long count = 0;
    while (a.finish == 0)
    {
        pthread_mutex_lock(&a.mutex);
        count = a.count;
        pthread_mutex_unlock(&a.mutex);
        printf("%ld\n", count);
        sleep(1);
    }

    pthread_join(thread, NULL);

    pthread_mutex_destroy(&a.mutex);
    return EXIT_SUCCESS;
}
