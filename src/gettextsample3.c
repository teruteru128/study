
#include <mytextdomain.h>
#include "gettextsample3.h"

int main(void){
	int microseconds = 3.75 * 1000000;
	inittextdomain();

	printf(_("%dmicro seconds stopping.\n"), microseconds);
	usleep(microseconds);
	printf(_("%dmicro seconds stoped.\n"), microseconds);
	
	return 0;
}
