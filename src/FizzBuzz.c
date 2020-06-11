
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "printint.h"

void showFizzBuzz()
{
  int n;
  int tmp;
  for (n = 1; n <= 40; n++)
  {
    if (n % 3 == 0 && n % 5 == 0)
    {
      printf("Fizz Buzz\n");
    }
    else if (n % 3 == 0)
    {
      printf("Fizz\n");
    }
    else if (n % 5 == 0)
    {
      printf("Buzz\n");
    }
    else
    {
      printf("%d\n", n);
    }
  }
}

#define MAX (100000000)
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  showFizzBuzz();
}
