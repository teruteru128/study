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
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

void *func(void *i)
{
    int j;

    for (j = 0; j < 1000000; j++)
        atomic_fetch_add((int *)i, 1); // counts up.

    return NULL;
}

void mode1()
{
    int n = 0, x = 0;
    pthread_t th[10];

    for (n = 0; n < 10; n++)
        pthread_create(&th[n], NULL, func, &x);

    for (n = 0; n < 10; n++)
        pthread_join(th[n], NULL);

    printf("%d\n", x);
}

void *func2(void *i)
{
    int j;

    for (j = 0; j < 1000000; j++)
        (*((_Atomic volatile int *)i))++; // counts up.

    return NULL;
}

void mode2()
{
    int n = 0, x = 0;
    pthread_t th[10];

    for (n = 0; n < 10; n++)
        pthread_create(&th[n], NULL, func2, &x);

    for (n = 0; n < 10; n++)
        pthread_join(th[n], NULL);

    printf("%d\n", x);
}

int main(int argc, char *argv[], char *envp[])
{
    if (argc < 2)
    {
        return 0;
    }
    int mode = (int)strtol(argv[1], NULL, 10);

    switch (mode)
    {
    case 1:
        mode1();
        break;
    case 2:
        mode2();
        break;
    default:
        fprintf(stderr, "unknown mode: %d\n", mode);
        return 1;
    }

    return 0;
}
