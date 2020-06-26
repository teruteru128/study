#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define N_THREAD 1000

#define MAX_PRIME_NUMBERS 1000000

int primeNumber[MAX_PRIME_NUMBERS];
int nPrimeNumber = 0;

int isPrimeNumber(int m) {
    int i;
    for(i = 0; i < nPrimeNumber; i++) {
        if(m % primeNumber[i] == 0) {
            return 0;
        }
    }
    return 1;
}

int countPrimeNumbers(int n) {
    int i;
    nPrimeNumber = 0;
    for(i = 2; i<= n; i++) {
        if(isPrimeNumber(i)) {
            if(nPrimeNumber >= MAX_PRIME_NUMBERS){
                printf("Oops, too many prime numbers.\n");
                exit(1);
            }
            primeNumber[nPrimeNumber] = i;
            nPrimeNumber++;
        }
    }
    return nPrimeNumber;
}

void *threadFunc(void *arg) {
    int n = (int) arg;
    int x;

    x = countPrimeNumbers(n);
    if(x != 25) {
        printf("Number of prime numbers under %d is %d !?\n", n, x);
    }

    return NULL;
}

int main() {
    pthread_t threads[N_THREAD];
    int i;

    for(i = 0; i < N_THREAD; i++) {
        if(pthread_create(&threads[i], NULL, threadFunc, (void *)100) != 0){
            printf("Can't create thread(%d)\n", i);
            exit(1);
        }
    }

    for(i = 0; i < N_THREAD; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Done\n");

    return 0;
}
