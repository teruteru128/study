
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char **argv)
{
    syslog(LOG_DEBUG, "💩");
    return EXIT_SUCCESS;
}
