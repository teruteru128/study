
#include "mytextdomain.h"
#include "gettextsample3.h"

#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
  int microseconds = 3.75 * 1000000;
  inittextdomain();

  printf(_("%dmicro seconds stopping.\n"), microseconds);
  usleep(microseconds);
  printf(_("%dmicro seconds stoped.\n"), microseconds);

  return 0;
}
