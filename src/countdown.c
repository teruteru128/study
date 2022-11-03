
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int countdowns(struct tm *tm)
{
    time_t a = mktime(tm);
    time_t now = 0;
    double diff = 0;
    while ((diff = difftime(a, now = time(NULL))) > 0)
    {
        localtime_r(&now, tm);
        printf("%04d/%02d/%02d %02d:%02d:%02d(%ld)\n", tm->tm_year + 1900,
               tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
               tm->tm_sec, (long)diff);
        sleep(1);
    }
    fputs("どーん！\n", stdout);
    return 0;
}
