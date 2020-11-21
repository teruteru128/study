
#include "mytextdomain.h"
#include "gettextsample3.h"

#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#define TIMEFD_ENABLE 1
#if TIMEFD_ENABLE
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <err.h>
#include <sys/timerfd.h>
#endif

int main(void)
{
  useconds_t microseconds = 3.75 * 1000000;
#if TIMEFD_ENABLE
  struct itimerspec spec;
  spec.it_value.tv_sec = microseconds / 1000000;
  spec.it_value.tv_nsec = (microseconds % 1000000) * 1000;
  spec.it_interval.tv_sec = 0;
  spec.it_interval.tv_nsec = 0;
#endif
  inittextdomain();
#if TIMEFD_ENABLE
  int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
  if (timerfd < 0)
    err(EXIT_FAILURE, "timerfd");

  int ret = timerfd_settime(timerfd, 0, &spec, NULL);
  if (ret != 0)
  {
    perror("timerfd_settime");
    close(timerfd);
    exit(EXIT_FAILURE);
  }
#endif

  printf(_("%dmicro seconds stopping.\n"), microseconds);
#if TIMEFD_ENABLE
  // expiration
  uint64_t exp;
  ret = read(timerfd, &exp, sizeof(exp));
  if (ret != 0)
  {
    perror("read");
    close(timerfd);
    exit(EXIT_FAILURE);
  }
#else
  usleep(microseconds);
#endif
  printf(_("%dmicro seconds stoped.\n"), microseconds);

#if TIMEFD_ENABLE
  close(timerfd);
#endif
  return EXIT_SUCCESS;
}
