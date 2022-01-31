
#include <stdio.h>
#include <time.h>
#include "timeutil.h"

void leapyeartest()
{
    char buf[BUFSIZ];
    struct tm uruu1;
    uruu1.tm_sec = 0;
    uruu1.tm_min = 0;
    uruu1.tm_hour = 0;
    uruu1.tm_mday = 29;
    uruu1.tm_mon = 1;
    uruu1.tm_year = 2020 - 1900;
    uruu1.tm_wday = 0;
    uruu1.tm_yday = 0;
    uruu1.tm_isdst = 0;
    time_t t = mktime(&uruu1);
    struct tm uruu2;
    localtime_r(&t, &uruu2);
    strftime(buf, BUFSIZ, "%FT%T+09:00", &uruu2);
    printf("%s, %d\n", buf, tmcomp(&uruu1, &uruu2));
}
