#include "001.h"
#include <stdio.h>

static void initGettext(){
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	fputs(LOCALEDIR, stdout);
	textdomain (PACKAGE);
}

int helloWorld001(){
	initGettext();
	printf(_("Hello world!\n"));
}

