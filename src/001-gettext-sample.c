#include "001.h"

static void initGettext(){
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALDIR);
	puts(LOCALDIR);
	textdomain (PACKAGE);
}

int helloWorld001(){
	initGettext();
	printf(_("Hello world!\n"));
}

