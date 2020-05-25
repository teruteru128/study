
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define D_SIZE (1024)

int main(int argc, char *argv[])
{
  int i = 0;
  for (; i < 10; i++)
  {
    printf("%d\n", i);
  }
  printf("%d\n", i);
  return EXIT_SUCCESS;
}
