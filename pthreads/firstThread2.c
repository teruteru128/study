
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
/*
Linuxとpthreadsによるマルチスレッドプログラミング入門 - 渋谷克智
*/
void anotherFunc(int n)
{
  if (n == 2)
  {
    printf("Hasta la vista, baby.\n");
    exit(0);
  }
}

void *threadFunc(void *arg)
{
  int i;
  for (i = 0; i < 3; i++)
  {
    printf("I'm threadFunc.(%d)\n", i);
    anotherFunc(i);
    sleep(1);
  }

  return NULL;
}

int main(int argc, char *argv[])
{
  pthread_t thread;
  int i = 0;

  if (pthread_create(&thread, NULL, threadFunc, NULL) != 0)
  {
    perror("failed to create new thread");
    return EXIT_FAILURE;
  }

  for (i = 0; i < 5; i++)
  {
    printf("I'm main. (%d)\n", i);
    sleep(1);
  }

  return EXIT_SUCCESS;
}
