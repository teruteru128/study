/**
 * @file atomicsample.c
 * @author https://blog.bitmeister.jp/?p=4561
 * @brief 
 * @version 0.1
 * @date 2018-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

_Atomic int i = ATOMIC_VAR_INIT(0);

void *func() {
    int j;

    for (j = 0; j < 1000000; j++)
        i++;

    return NULL;
}

int main() {
    int n = 0;
    pthread_t th[10];

    for (n = 0; n < 10; n++)
        pthread_create(&th[n], NULL, func, NULL);

    for (n = 0; n < 10; n++)
        pthread_join(th[n], NULL);

    printf("%d\n", i);

    return 0;
}
