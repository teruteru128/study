
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "study-config.h"

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Help me!\n"));
  return EXIT_SUCCESS;
}
