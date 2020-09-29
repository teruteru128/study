
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define SIZE 16777216

struct a
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

void *threadFunc(void *arg)
{
  struct a *a = (struct a *)arg;
  double table[SIZE];
  int i;

  for (i = 0; i < SIZE; i++)
  {
    table[i] = i * 3.14;
  }
  printf("準備完了\n");
  pthread_mutex_lock(&a->mutex);
  pthread_cond_wait(&a->cond, &a->mutex);
  pthread_mutex_unlock(&a->mutex);
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
  struct a a;

  pthread_attr_init(&attr);
  pthread_mutex_init(&a.mutex, NULL);
  pthread_cond_init(&a.cond, NULL);

  size_t stacksize;
  pthread_attr_getstacksize(&attr, &stacksize);
  printf("%lu -> ", stacksize);

  stacksize += SIZE * sizeof(double);
  printf("%lu\n", stacksize);

  if (pthread_attr_setstacksize(&attr, stacksize) != 0)
  {
    perror("Failed to set stack size.");
    return EXIT_FAILURE;
  }

  if (pthread_create(&thread, &attr, threadFunc, &a) != 0)
  {
    perror("Error: Failed to create new thread.");
    return EXIT_FAILURE;
  }

  printf("エンターキーを押して終了...\n");
  fgetc(stdin);
  pthread_cond_signal(&a.cond);

  if (pthread_join(thread, NULL) != 0)
  {
    perror("Error: Failed to wait for the thread termination.");
    return EXIT_FAILURE;
  }

  pthread_mutex_destroy(&a.mutex);
  pthread_cond_destroy(&a.cond);
  pthread_attr_destroy(&attr);
  return EXIT_SUCCESS;
}
