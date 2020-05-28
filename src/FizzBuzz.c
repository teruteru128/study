
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

void showNabeatsu()
{
  int n;
  char txt[64] = {0};
  for (n = 1; n <= 40; n++)
  {
    itoa(n, txt, 10);
    if (n % 3 == 0 || strchr(txt, '3'))
    {
      fputs("アホ\n", stdout);
    }
    else
    {
      fputs(txt, stdout);
      fputs("\n", stdout);
    }
    memset(txt, 0, 64);
  }
}
#define MAX (100000000)
int main(int argc, char *argv[])
{
  int a = 334;
  char b[64];
  itoa(a, b, 10);
  fputs(b, stdout);
  fputs("\n", stdout);
  int i = 0, max = 1000000;
  struct timespec start;
  FILE *devnull = fopen("/dev/null", "w");
  clock_gettime(CLOCK_REALTIME, &start);
  if (devnull == NULL)
  {
    return EXIT_FAILURE;
  }
  for (; i < MAX; i++)
  {
    itoa(i, b, 10);
  }
  struct timespec end;
  clock_gettime(CLOCK_REALTIME, &end);
  fclose(devnull);
  time_t sec = end.tv_sec - start.tv_sec;
  long nsec = end.tv_nsec - start.tv_nsec;
  double passed = sec * 1000000000L + nsec;
  printf("%.8f秒かかりました\n", passed / 1e9);
  printf("1秒あたり %.12f回\n", MAX / (passed / 1e9));
  if (strstr(argv[0], "Nabeatsu"))
  {
    showNabeatsu();
  }
  else if (strstr(argv[0], "FizzBuzz"))
  {
    showFizzBuzz();
  }
}
