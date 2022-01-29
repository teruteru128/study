
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile atomic_int flag = 0;
static pthread_t calledthread = 0;
static pthread_t mainthread = 0;

/**
 * signal sample
 * https://www.jpcert.or.jp/sc-rules/c-sig30-c.html
 * signalハンドラ内で fprintf は未定義動作
 * TODO: use sigaction(2)
 */
static void sigint_action(int sig)
{
    // fprintf(stdout, "SIGINT recive(%d)\n", sig);
    flag = sig;
    calledthread = pthread_self();
}

static int shutdown = 1;

static void *signal_catcher(void *arg)
{
    int sigc = 0;
    while (shutdown)
    {
        if (flag != 0)
        {
            sigc = atomic_exchange(&flag, 0);
            fprintf(stdout, "SIGINT recive(%d), %lu, %lu, %lu\n", sigc, mainthread, pthread_self(), calledthread);
        }
        usleep(250000);
    }
    return NULL;
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
    pid_t pid = getpid();
    mainthread = pthread_self();
    printf("pid : %d, %lu\n", pid, mainthread);
    int rc = set_signal_handler();
    if (rc != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }
    printf("set signal handler ok\n");
    pthread_t catcher = 0;
    pthread_create(&catcher, NULL, signal_catcher, NULL);
    pthread_detach(catcher);
    int ret = 0;
    struct timespec req = { 15, 0 };
    struct timespec rem = { 0 };
    do
    {
        ret = nanosleep(&req, &rem);
        if (ret != 0)
        {
            printf("ret : %d, %2ld.%09ld\n", ret, rem.tv_sec, rem.tv_nsec);
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    } while (ret != 0);
    shutdown = 0;
    printf("sleep break\n");
    return 0;
}
