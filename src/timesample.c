
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    tzset();
    /* 次の実行日時を取得する */
    /* 現在時刻を取得する */
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    if (time(NULL) == currentTime.tv_sec)
    {
        printf("==\n");
    }
    else
    {
        printf("!=\n");
    }
    char timestrbuf[BUFSIZ] = "";
    struct tm tm;
    gmtime_r(&currentTime.tv_sec, &tm);
    printf("%d %04d-%02d-%02dT%02d:%02d:%02dZ\n", tm.tm_isdst,
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
           tm.tm_sec);
    struct tm *now = localtime_r(&currentTime.tv_sec, &tm);
    printf("%d %04d-%02d-%02dT%02d:%02d:%02d", tm.tm_isdst, tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef __USE_MISC
    printf("%c%02ld:%02ld %ld, %s", tm.tm_gmtoff < 0 ? '-' : '+',
           tm.tm_gmtoff / 3600, tm.tm_gmtoff % 3600, tm.tm_gmtoff, tm.tm_zone);
#endif
    fputs("\n", stdout);
    strftime(timestrbuf, BUFSIZ, "%FT%T%z %Z", &tm);
    printf("%d %s\n", tm.tm_isdst, timestrbuf);
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 4;
    tm.tm_mday = 25;
    tm.tm_mon = 11;
    printf("%d %04d-%02d-%02dT%02d:%02d:%02d", tm.tm_isdst,
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
           tm.tm_sec);
#ifdef __USE_MISC
    printf("%c%02ld:%02ld %ld, %s", tm.tm_gmtoff < 0 ? '-' : '+',
           tm.tm_gmtoff / 3600, tm.tm_gmtoff % 3600, tm.tm_gmtoff, tm.tm_zone);
#endif
    fputs("\n", stdout);
    strftime(timestrbuf, BUFSIZ, "%FT%T%z %Z", &tm);
    printf("%d %s\n", tm.tm_isdst, timestrbuf);
    time_t time1 = mktime(now);
    double diff = difftime(time1, currentTime.tv_sec);
    int catch = 0;
    printf("クリスマスまであと%ld秒, %.12lf日\n%n", (long)diff, diff / 86400L,
           &catch);
    printf("%d\n", catch);
    // printf("@%ld\n", time1);
    return EXIT_SUCCESS;
}
