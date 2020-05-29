
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
/*
Linuxとpthreadによるマルチスレッドプログラミング入門 - 渋谷克智
3-2
*/
long varA;
void *threadFunc(void *arg)
{
  long n = (long)arg;
  long varB;

  varB = 4 * n;
  printf("threadFunc-%ld-1: varA=%ld, varB=%ld\n", n, varA, varB);
  varA = 5 * n;
  printf("threadFunc-%ld-2: varA=%ld, varB=%ld\n", n, varA, varB);
  sleep(2);
  printf("threadFunc-%ld-3: varA=%ld, varB=%ld\n", n, varA, varB);
  varB = 6 * n;
  printf("threadFunc-%ld-4: varA=%ld, varB=%ld\n", n, varA, varB);
  return NULL;
}

int main(int argc, char *argv[])
{
  pthread_t thread1, thread2;
  long varB = 0;

  varA = 1;
  varB = 2;
  printf("main-1: varA=%ld, varB=%ld\n", varA, varB);
  pthread_create(&thread1, NULL, threadFunc, (void *)1);
  sleep(1);
  varB = 3;
  printf("main-2: varA=%ld, varB=%ld\n", varA, varB);
  pthread_create(&thread2, NULL, threadFunc, (void *)2);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  printf("main-3: varA=%ld, varB=%ld\n", varA, varB);

  return EXIT_SUCCESS;
}
