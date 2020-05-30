
#include "study-config.h"
#include <stdio.h>
#include "localetest1.h"

int main(int argc, char **argv)
{
  char *locale = setlocale(LC_ALL, "");
  printf("%p : %s\n", locale, locale);
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Hello world!\n"));
  return 0;
}
