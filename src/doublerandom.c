
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <limits.h>

int main(argc, argv) int argc;
char **argv;
{
  FILE *in = fopen("/dev/urandom", "rb");
  if (!in)
  {
    perror("fopen");
    return EXIT_FAILURE;
  }
  unsigned int seed;
  size_t k = fread(&seed, sizeof(unsigned int), 1, in);
  if (k != 1)
  {
    perror("fopen");
    fclose(in);
    return EXIT_FAILURE;
  }
  fclose(in);
  srandom(seed);
  // 31bits
  unsigned long l = (unsigned long)random();
  // 62bits
  l = (l << 31) | (unsigned long)random();
  double r = (double)l / ((1UL << 62) - 1);
  printf("%.24lf\n", r);
  return EXIT_SUCCESS;
}
