
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

#include "server.h"

static void usage(int status)
{
    fprintf(stderr, "argument count mismatch error.\nplease input a service name or port number.\n");
    exit(status);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(EXIT_FAILURE);
    }
    if (init_server(argv[1]) == -1)
    {
        fprintf(stderr, "init_server failure.\n");
        exit(EXIT_FAILURE);
    }
    do_service();
    close_server();
    return 0;
}
