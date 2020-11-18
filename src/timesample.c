
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    /* 次の実行日時を取得する */
    /* 現在時刻を取得する */
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    struct tm tm;
    tzset();
    gmtime_r(&currentTime.tv_sec, &tm);
    printf("%d %04d-%02d-%02dT%02d:%02d:%02dZ\n", tm.tm_isdst, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    localtime_r(&currentTime.tv_sec, &tm);
    printf("%d %04d-%02d-%02dT%02d:%02d:%02d+09:00\n", tm.tm_isdst, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef __USE_MISC
    printf("%ld, %s\n", tm.tm_gmtoff, tm.tm_zone);
#endif
    tm.tm_sec = 0;
    tm.tm_min = 10;
    tm.tm_hour = 10;
    tm.tm_mday = 10;
    tm.tm_mon = 9;
    time_t time1 = mktime(&tm);
    tm.tm_sec = 0;
    tm.tm_min = 10;
    tm.tm_hour = 10;
    tm.tm_mday = 10;
    tm.tm_mon = 9;
    tm.tm_year = 2010 - 1900;
    time_t time2 = mktime(&tm);
    printf("%lf\n", difftime(time1, time2));
    return EXIT_SUCCESS;
}
