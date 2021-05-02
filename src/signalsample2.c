
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>

/**
 * signal sample
 * */
static void sigint_action(int sig)
{
    fprintf(stderr, "sigint : %d\n", sig);
}

/**
 * signal sample
 * */
static void sighup_action(int sig)
{
    fprintf(stderr, "config reloading...%d\n", sig);
}

int main(int argc, char **argv)
{
    struct sigaction act[2];
    act[0].sa_handler = sigint_action;
    act[1].sa_handler = sighup_action;
    if (sigaction(SIGINT, &act[0], NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGHUP, &act[1], NULL) != 0)
    {
        perror("sigaction(SIGINT)");
        return EXIT_FAILURE;
    }
    /*
    if (signal(SIGINT, sigint_action) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }
    if (signal(SIGINT, sigint_action) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }
    */
    printf("started\n");
    int c = 0;
    c = fgetc(stdin);
    if (ferror(stdin))
    {
        perror(NULL);
    }
    if (feof(stdin))
    {
        printf("eof\n");
    }
    printf("%08x\n", c);
    return (EXIT_SUCCESS);
}
