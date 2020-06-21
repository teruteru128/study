
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define URANDOM_PATH "/dev/urandom"
#define BUF_SIZE 8
#include "random.h"

int orz(int num)
{
  int count = num > 0 ? num : 334;
  uint32_t seed;
  if (read_file(URANDOM_PATH, &seed, sizeof(uint32_t), 1) != 0)
  {
    warnx("failed");
    return EXIT_FAILURE;
  }
  size_t i = 0;

  char *messages[] = {
    "orz",
    "申し訳ございませんでした",
    "ごめんなさい",
    "すみませんでした",
    NULL};
  size_t messages_size = 0;
  char **tmp = messages;
  while (*tmp++ != NULL)
  {
    messages_size++;
  }
  for (i = 0; i < count; i++)
  {
    printf("%s\n", messages[(seed = xorshift(seed)) % messages_size]);
  }

  return EXIT_SUCCESS;
}
