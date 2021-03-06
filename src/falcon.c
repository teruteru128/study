
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <random.h>
#include <xorshift.h>

#define URANDOM_PATH "/dev/urandom"
#define MAX 1000000

uint32_t seed = 0;

static void shuffle(char (*array)[4], size_t size)
{
  char w[4] = "";
  for (size_t i = 0; i < size; i++)
  {
    size_t j = (seed = xorshift(seed)) % size;
    memcpy(w, array[i], 3);
    memcpy(array[i], array[j], 3);
    memcpy(array[j], w, 3);
  }
}

int main(int argc, char **argv)
{
  // /dev/urandom から8192バイトも読み込むことないよね？
  if (read_file(URANDOM_PATH, &seed, sizeof(uint32_t), 1) != 0)
  {
    perror("failed");
    return EXIT_FAILURE;
  }

  char messages[][4] = {"フ", "ァ", "ル", "コ", "ン", "・", "パ", "ン", "チ", ""};
  size_t messages_size = 0;
  while (messages[messages_size][0] != '\0')
  {
    messages_size++;
  }
  size_t i = 0;
  size_t j = 0;
  for (; j < MAX; j++)
  {
    shuffle(messages, messages_size);
    for (i = 0; i < messages_size; i++)
    {
      fputs(messages[i], stdout);
      // fputs(messages[(seed = xorshift(seed)) % messages_size], stdout);
    }
    fputs("\n", stdout);
  }

  return EXIT_SUCCESS;
}
