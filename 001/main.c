
#include "main.h"

static void initGettext(){
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	puts(LOCALEDIR);
	textdomain (PACKAGE);
}

int main(int argc, char** argv){
	initGettext();
	printf(_("Hello world!\n"));
	return 0;
}

