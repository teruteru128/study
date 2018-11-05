
#include "main026.h"

int main(int argc, char** argv){
  setlocale (LC_ALL, "");
  bindtextdomain ("study", LOCALEDIR);
  textdomain ("study");
  printf(_("Hello world!\n"));
  return 0;
}

