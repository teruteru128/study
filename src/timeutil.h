
#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <time.h>
struct timespec *difftimespec(struct timespec *result, struct timespec *time1, struct timespec *time0);

#endif
