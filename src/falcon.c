
#include "config.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "random.h"

#define URANDOM_PATH "/dev/urandom"
#define MAX 1000000

uint32_t seed = 0;

void shuffle(void **array, size_t size)
{
  void *t = NULL;
  for (int i = 0; i < size; i++)
  {
    int j = (seed = xorshift(seed)) % size;
    t = array[i];
    array[i] = array[j];
    array[j] = t;
  }
}

int main(int argc, char **argv)
{
  int len = 4;
  // /dev/urandom から8192バイトも読み込むことないよね？
  if (read_file(URANDOM_PATH, &seed, sizeof(uint32_t), 1) != 0)
  {
    perror("failed");
    return EXIT_FAILURE;
  }

  char *messages[] = {
      "フ", "ァ", "ル", "コ", "ン", "・", "パ", "ン", "チ",
      NULL};
  size_t messages_size = 0;
  char **tmp = messages;
  for (; *tmp++ != NULL; messages_size++)
  {
  }
  size_t i = 0;
  size_t j = 0;
  for (; j < MAX; j++)
  {
    shuffle((void **)messages, messages_size);
    for (i = 0; i < messages_size; i++)
    {
      fputs(messages[i], stdout);
      // fputs(messages[(seed = xorshift(seed)) % messages_size], stdout);
    }
    fputs("\n", stdout);
  }

  return EXIT_SUCCESS;
}
