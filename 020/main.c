
#include "main.h"

int main(int argc, char** argv){
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALDIR);
  textdomain (PACKAGE);
  printf(_("Hello world!\n"));
  return 0;
}
