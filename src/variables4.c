#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* varA;
#define STRINGSIZE 32

void *threadFunc(void* arg){
    int n = (int)arg;
    snprintf(varA, STRINGSIZE, "Hello I'm No.%d", n);
    printf("threadFunc-%d: Sets varA as '%s'\n", n, varA);
    sleep(2);
    printf("threadFunc-%d: After 2 secs. varA is '%s'\n", n, varA);

    return NULL;
}

int main(int argc, char* argv[]){
    pthread_t thread1, thread2;
    char varB[STRINGSIZE];
    varA = varB;

    strcpy(varA, "Good morning");
    printf("main-1: varA is '%s', varB is '%s'\n", varA, varB);
    pthread_create(&thread1, NULL, threadFunc, (void*)1);
    sleep(1);
    printf("main-2: varA is '%s', varB is '%s'\n", varA, varB);
    pthread_create(&thread2, NULL, threadFunc, (void*)2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("main-3: varA is '%s', varB is '%s'\n", varA, varB);
    return EXIT_SUCCESS;
}
