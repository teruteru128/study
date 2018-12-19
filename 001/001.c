#include "001.h"

static void initGettext(){
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	puts(LOCALEDIR);
	textdomain (PACKAGE);
}

int helloWorld001(){
	initGettext();
	printf(_("Hello world!\n"));
}

