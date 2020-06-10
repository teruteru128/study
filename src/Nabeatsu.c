
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "printint.h"

void showNabeatsu()
{
  int n;
  char txt[64];
  for (n = 1; n <= 40; n++)
  {
    memset(txt, 0, 64);
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
  }
}
#define MAX (100000000)
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  if (strstr(argv[0], "Nabeatsu"))
  {
    showNabeatsu();
  }
  else
  {
    int a = 334;
    char b[64];
    itoa(a, b, 10);
    fputs(b, stdout);
    fputs("\n", stdout);
    int i = 0, max = 1000000;
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    for (; i < MAX; i++)
    {
      itoa(i, b, 10);
    }
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    time_t sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    double passed = (sec * 1e9) + nsec;
    double seconds = passed / 1e9;
    fprintf(stderr, _("It took %.8f seconds.\n"), seconds);
    fprintf(stderr, _("%.1f times per second\n"), MAX / seconds);
    fprintf(stderr, _("%.12f seconds per time\n"), seconds / MAX);
  }
}
