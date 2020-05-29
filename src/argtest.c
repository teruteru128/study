
#include <stdio.h>
#include <stdlib.h>

void init(void)
{
  return;
}

typedef struct args_t
{
} args_t;

void parse_args(struct args_t *args, int argc, char *argv[])
{
  return;
}

int main(int argc, char *argv[])
{
  args_t args;
  init();
  parse_args(&args, argc, argv);
  return EXIT_SUCCESS;
}
