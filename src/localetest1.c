
#include "localetest1.h"

int main(int argc, char** argv){
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  printf(_("Hello world!\n"));
  return 0;
}

