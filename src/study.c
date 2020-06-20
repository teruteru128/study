
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gettextsample.h"
#include "printint.h"
#include "random.h"
#include "bitset.h"
#include "orz.h"

/**
 * --version
 * --help
 * 
 * orz
 */
int main(int argc, char *argv[])
{
  //setlocale(LC_ALL, "");
  //bindtextdomain(PACKAGE, LOCALEDIR);
  //textdomain(PACKAGE);
  //printf(_("Help me!\n"));
  //orz(1);

  int i = 0;
  int under = 62;
  double topsubunder = 0;
  for(; i < 95; i++)
  {
    topsubunder = 10 + 2.5 * i;
    printf("%c : %.1fcm ( + %.1f cm)\n", i >= 26 ? '?' : i + 'A', under + topsubunder, topsubunder);
  }
  return EXIT_SUCCESS;
}
