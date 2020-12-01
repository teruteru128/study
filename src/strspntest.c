
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR "abcdefgabcdefghij"

int main(int argc, char **argv)
{
  int c = 0;
  char *v = NULL;
  for (c = 1; c < argc; c++)
  {
    v = argv[c];
    printf("%s\n", v);
  }
  char search[21];

  printf("文字群を入力しなさい。\n");
  //int r = fscanf(stdin, "%20s", search);
  //if (r == EOF)
  char *r = fgets(search, 21, stdin);
  if(r == NULL)
  {
    perror("scanf");
    return EXIT_FAILURE;
  }

  size_t p = strspn(STR, search);
  printf("%zd\n", p);
  return EXIT_SUCCESS;
}
