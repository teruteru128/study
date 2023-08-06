
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "timerfdsample0.h"
#include "timerfdsample1.h"
#include "timerfdsample2.h"
#include "timerfdsample3.h"
#include "timerfdsample4.h"
#include "timerfdsample5.h"
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        fputs("timerfdsample <number>\n", stderr);
        return EXIT_FAILURE;
    }

    errno = 0;
    long command = strtol(argv[1], NULL, 10);
    if (errno != 0)
    {
        perror("strtol");
        return EXIT_FAILURE;
    }

    int ret;
    switch (command)
    {
    case 0:
        ret = timerfdsample0();
        break;
    case 1:
        ret = timerfdsample1();
        break;
    case 2:
        ret = timerfdsample2();
        break;
    case 3:
        ret = timerfdsample3(argc, argv);
        break;
    case 4:
        ret = timerfdsample4();
        break;
    case 5:
        ret = timerfdsample5();
        break;
    default:
        fputs("unknown command number\n", stderr);
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}
