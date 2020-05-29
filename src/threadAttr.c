
#include "config.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define SIZE 10000000

void *threadFunc(void *arg)
{
  double table[SIZE];
  int i;

  for (i = 0; i < SIZE; i++)
  {
    table[i] = i * 3.14;
  }
  for (i = 0; i < SIZE; i++)
  {
    printf("%lf\n", table[i]);
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  pthread_attr_t attr;
  pthread_t thread;

  pthread_attr_init(&attr);

  if (pthread_attr_setstacksize(&attr, SIZE * sizeof(double) + 100000) != 0)
  {
    perror("Failed to set stack size.");
    return EXIT_FAILURE;
  }

  if (pthread_create(&thread, &attr, threadFunc, NULL) != 0)
  {
    perror("Error: Failed to create new thread.");
    return EXIT_FAILURE;
  }

  if (pthread_join(thread, NULL) != 0)
  {
    perror("Error: Failed to wait for the thread termination.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
