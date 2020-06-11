
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "printint.h"

#define MAX (100000000L)
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  int a = 0x114514;
  char b[64];
  itoa(a, b, 10);
  fputs(b, stdout);
  fputs("\n", stdout);
  long i = 0;
  struct timespec start;
  struct timespec end;
  time_t sec;
  long nsec;
  double passed;
  double seconds;
  clock_gettime(CLOCK_REALTIME, &start);
  for (i = 0; i < MAX; i++)
  {
    ltoa(a, b, 10);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  sec = end.tv_sec - start.tv_sec;
  nsec = end.tv_nsec - start.tv_nsec;
  passed = (sec * 1e9) + nsec;
  seconds = passed / 1e9;
  fprintf(stderr, _("It took %.8f seconds.\n"), seconds);
  fprintf(stderr, _("%.1f times per second\n"), MAX / seconds);
  fprintf(stderr, _("%.12f seconds per time\n"), seconds / MAX);
  snprintInt64(b, 64, a);
  fputs(b, stdout);
  fputs("\n", stdout);
  clock_gettime(CLOCK_REALTIME, &start);
  for (i = 0; i < 5000000000L; i++)
  {
    snprintInt64(b, 64, a);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  sec = end.tv_sec - start.tv_sec;
  nsec = end.tv_nsec - start.tv_nsec;
  passed = (sec * 1e9) + nsec;
  seconds = passed / 1e9;
  fprintf(stderr, _("It took %.8f seconds.\n"), seconds);
  fprintf(stderr, _("%.1f times per second\n"), 5000000000 / seconds);
  fprintf(stderr, _("%.12f seconds per time\n"), seconds / 5000000000);
}
