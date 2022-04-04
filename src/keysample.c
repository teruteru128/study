
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_key_t key;

/**
 * @brief func と同じスレッドで呼び出される
 * 
 * @param buf 
 */
static void destructor(void *buf)
{
    printf("dest: (%lu) %s\n", pthread_self(), (char *)buf);
    free(buf);
}

static atomic_long count = 0;
#define TEXT_LENGTH 64

/**
 * @brief 
 * 
 * @param arg 
 * @return void* 
 */
void *func(void *arg)
{
    (void)arg;
    char *buf = malloc(TEXT_LENGTH);
    snprintf(buf, TEXT_LENGTH, "うんちー！%ld", count++);
    printf("func: (%lu) %s\n", pthread_self(), buf);
    pthread_setspecific(key, buf);
    usleep(500000);
    return NULL;
}

#define THREADS 128UL

/**
 * @brief スレッド固有データのサンプル
 * EVP_MD_CTXとかをセットするんやろな、本来は
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
    printf("Hello! This is %lu!\n", pthread_self());
    pthread_key_create(&key, destructor);
    pthread_t threads[THREADS];
    int ret = 0;
    for (size_t i = 0; i < THREADS; i++)
    {
        threads[i] = 0;
        ret = pthread_create(&threads[i], NULL, func, NULL);
        if (ret != 0)
        {
            break;
        }
    }
    for (size_t i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_key_delete(key);
    return 0;
}
