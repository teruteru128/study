
#include "main.h"

int main(int argc, char** argv){
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	puts(LOCALEDIR);
	textdomain (PACKAGE);
	printf(_("Hello world!\n"));
	return 0;
}

