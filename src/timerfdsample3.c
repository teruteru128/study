
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    if ((argc != 1) && (argc != 2) && (argc != 3) && (argc != 4))
    {
        return EXIT_FAILURE;
    }
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    struct itimerspec spec;
    if (argc < 2)
    {
        // argc == 1
        spec.it_value.tv_sec = 1;
    }
    else
    {
        // (argc == 2) || (argc == 3) || (argc == 4)
        spec.it_value.tv_sec = strtol(argv[1], NULL, 0);
    }
    spec.it_value.tv_nsec = 0;
    if (argc < 3)
    {
        // (argc == 1) || (argc == 2)
        spec.it_interval.tv_sec = 1;
    }
    else
    {
        // (argc == 3) || (argc == 4)
        spec.it_interval.tv_sec = strtol(argv[2], NULL, 0);
    }
    spec.it_interval.tv_nsec = 0;
    size_t max_exp;
    if (argc < 4)
    {
        // (argc == 1) || (argc == 2) || (argc == 3)
        max_exp = 10;
    }
    else
    {
        // argc == 4
        max_exp = strtoul(argv[3], NULL, 0);
    }
    int r = timerfd_settime(timerfd, 0, &spec, NULL);
    if (r != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        return EXIT_FAILURE;
    }

    uint64_t expiredTimesNumber = 0;
    ssize_t ret = 0;
    int hasError = 0;
    size_t i = 0;
    while (i < max_exp)
    {
        ret = read(timerfd, &expiredTimesNumber, sizeof(uint64_t));
        if (ret != sizeof(uint64_t))
        {
            perror("recv");
            hasError = 1;
            break;
        }
        printf("うんちー！ %zu, %" PRIx64 "\n", i, expiredTimesNumber);
        i += expiredTimesNumber;
    }
    close(timerfd);
    printf("なんすかこれ\n");
    return hasError != 0;
}
