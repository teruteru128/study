
#include <stdio.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    struct tm tm = {0};
    tm.tm_year = 2021 - 1900;
    tm.tm_mon = 10;
    tm.tm_mday = 45;
    tm.tm_hour = 14;
    tm.tm_min = 19;
    tm.tm_sec = 19;

    time_t time = 0;

    time = mktime(&tm);

    localtime_r(&time, &tm);

    char buf[64];
    strftime(buf, 64, "%FT%T+09:00", &tm);

    printf("%s\n", buf);

    printf("%lf\n", 0x1.ap-5);
    // 000111.000
    //   0001.11000
    //   1.00011
    //
    return 0;
}
