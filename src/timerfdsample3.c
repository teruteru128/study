
#define _GNU_SOURCE 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "timerfdsample3.h"
#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

/**
 * @brief コマンドライン引数でtimerfdを制御できるようにした残骸
 *
 * @param argc
 * @param argv
 * @return int
 */
int timerfdsample3(int argc, const char *argv[])
{
    setlocale(LC_ALL, "");
    if (argc < 1 || 5 < argc)
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
    newSpec.it_interval.tv_sec = argc >= 3 ? strtol(argv[3], NULL, 0) : 1;
    newSpec.it_interval.tv_nsec = 0;
    newSpec.it_value.tv_sec = argc >= 2 ? strtol(argv[2], NULL, 0) : 1;
    newSpec.it_value.tv_nsec = 0;
    size_t max_exp = argc >= 5 ? strtoul(argv[4], NULL, 0) : 10;
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
        if (ret != sizeof(uint64_t) && ret != 0)
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
