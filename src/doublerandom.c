
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

int main(argc, argv)
  int argc;
  char* argv[];
{
  double r = (double)random() / RAND_MAX;
  printf("%lf\n", r);
  return EXIT_SUCCESS;
}
