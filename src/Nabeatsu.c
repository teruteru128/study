
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <printint.h>

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
  showNabeatsu();
}
