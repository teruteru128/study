
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * signal sample
 * https://www.jpcert.or.jp/sc-rules/c-sig30-c.html
 * signalハンドラ内で fprintf は未定義動作
 * TODO: use sigaction(2)
 */
static void sigint_action(int sig)
{
    //fprintf(stdout, "SIGINT recive(%d)\n", sig);
}

static int set_signal_handler(void)
{
    if (signal(SIGINT, sigint_action) == SIG_ERR)
    {
        printf("Error: signal() SIGINT: %s\n", strerror(errno));
        perror(NULL);
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    int rc = set_signal_handler();
    if (rc != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }
    printf("set signal handler ok\n");
    sleep(60);
    printf("sleep break\n");
    return 0;
}
