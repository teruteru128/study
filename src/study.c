
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
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Help me!\n"));
  orz(1);
  return EXIT_SUCCESS;
}
