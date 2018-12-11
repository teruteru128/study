
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>

static uint64_t *verifire_ptr = NULL;

static void sigint_action(int sig)
{
    fprintf(stderr, "verifier=%" PRIu64 ", %" PRIx64 "\n", *verifire_ptr, *verifire_ptr);
}

int main(int argc, char **argv)
{
    uint64_t verifire = 0;
    verifire_ptr = &verifire;
    if (signal(SIGINT, sigint_action) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }
    while (1)
    {
        verifire++;
    }
    return (EXIT_SUCCESS);
}
