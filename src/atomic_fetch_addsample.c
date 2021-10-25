/**
 * @file atomic_fetch_addsample.c
 * @author https://blog.bitmeister.jp/?p=4561
 * @brief 
 * @version 0.1
 * @date 2018-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>


void *func(void *i) {
    int j;

    for (j = 0; j < 1000000; j++)
        atomic_fetch_add((int *)i, 1); // counts up.

    return NULL;
}

int main() {
    int n = 0, x = 0;
    pthread_t th[10];

    for (n = 0; n < 10; n++)
        pthread_create(&th[n], NULL, func, &x);

    for (n = 0; n < 10; n++)
        pthread_join(th[n], NULL);

    printf("%d\n", x); 

    return 0;
}
