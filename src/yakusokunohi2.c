
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    struct tm a;
    a.tm_year = 2006 - 1900;
    a.tm_mon = 7;
    a.tm_mday = 16;
    a.tm_hour = 7;
    a.tm_min = 14;
    a.tm_sec = 22;

    time_t b;
    b = mktime(&a);
    localtime_r(&b, &a);

    char c[7][4] = { "日", "月", "火", "水", "木", "金", "土" };
    printf("%d %d %d %s\n", a.tm_year + 1900, a.tm_mon + 1, a.tm_mday,
           c[a.tm_wday]);
    return EXIT_SUCCESS;
}
