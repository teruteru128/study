/**
 * @file mutexsample.c
 * @author https://blog.bitmeister.jp/?p=4525
 * @brief 
 * @version 0.1
 * @date 2018-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <pthread.h>


struct params {
    int *i; 
    pthread_mutex_t *mut;
};

void *func(void *arg) {
    int j;
    struct params *prm = (struct params *) arg;

    for (j = 0; j < 1000000; j++) {
        pthread_mutex_lock(prm->mut);
        (*(int *)prm->i)++; // counts up
        pthread_mutex_unlock(prm->mut);
    }   

    return NULL;
}

int main() {
    int n = 0, x = 0;
    pthread_t th[10];
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
    struct params prm = {&x, &mut};

    for (n = 0; n < 10; n++)
        pthread_create(&th[n], NULL, func, &prm);

    for (n = 0; n < 10; n++)
        pthread_join(th[n], NULL);

    printf("%d\n", x); 

    return 0;
}
