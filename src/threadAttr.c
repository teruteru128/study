
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define SIZE 16777216

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

  size_t stacksize;
  pthread_attr_getstacksize(&attr, &stacksize);
  printf("%lu\n", stacksize);

  if (pthread_attr_setstacksize(&attr, SIZE * sizeof(double) + stacksize) != 0)
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

  pthread_attr_destroy(&attr);
  return EXIT_SUCCESS;
}
