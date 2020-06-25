
#include <stdio.h>
#include <stdlib.h>
#include <args_parse.h>

void init(void)
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
