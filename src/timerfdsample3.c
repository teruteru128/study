
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
    struct itimerspec newSpec;
    if (argc < 2)
    {
        // argc == 1
        newSpec.it_value.tv_sec = 1;
    }
    else
    {
        // (argc == 2) || (argc == 3) || (argc == 4)
        newSpec.it_value.tv_sec = strtol(argv[1], NULL, 0);
    }
    newSpec.it_value.tv_nsec = 0;
    if (argc < 3)
    {
        // (argc == 1) || (argc == 2)
        newSpec.it_interval.tv_sec = 1;
    }
    else
    {
        // (argc == 3) || (argc == 4)
        newSpec.it_interval.tv_sec = strtol(argv[2], NULL, 0);
    }
    newSpec.it_interval.tv_nsec = 0;
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
    struct itimerspec oldSpec;
    if (timerfd_settime(timerfd, 0, &newSpec, &oldSpec) != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        return EXIT_FAILURE;
    }
    printf("%lu.%lu, %lu.%lu\n", oldSpec.it_value.tv_sec, oldSpec.it_value.tv_nsec, oldSpec.it_interval.tv_sec, oldSpec.it_interval.tv_nsec);

    uint64_t expiredTimesNumber = 0;
    ssize_t ret = 0;
    int hasError = 0;
    uint64_t i = 0;
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
