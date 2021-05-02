
#include "mytextdomain.h"
#include "gettextsample4.h"

#include "config.h"
#include <stddef.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <sys/timerfd.h>

int main(void)
{
    useconds_t microseconds = 3.75 * 1000000;
    inittextdomain();

    struct itimerspec spec;
    spec.it_value.tv_sec = microseconds / 1000000;
    spec.it_value.tv_nsec = (microseconds % 1000000) * 1000;
    spec.it_interval.tv_sec = 0;
    spec.it_interval.tv_nsec = 0;
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (timerfd < 0)
        err(EXIT_FAILURE, "timerfd_create");

    if (timerfd_settime(timerfd, 0, &spec, NULL) != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        exit(EXIT_FAILURE);
    }

    printf(_("%dmicro seconds stopping.\n"), microseconds);
    // expiration
    uint64_t exp;
    ssize_t size = read(timerfd, &exp, sizeof(uint64_t));
    if (size != sizeof(uint64_t))
    {
        perror("read");
        close(timerfd);
        exit(EXIT_FAILURE);
    }
    printf(_("%dmicro seconds stoped.\n"), microseconds);

    close(timerfd);
    return EXIT_SUCCESS;
}
