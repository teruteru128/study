
#include <stdio.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    struct tm tm = { 0 };
    tm.tm_year = 2022 - 1900;
    tm.tm_mon = 1;
    tm.tm_mday = 22;
    time_t a = 0;
    a = mktime(&tm);
    while (localtime_r(&a, &tm)->tm_wday != 1)
    {
        tm.tm_year++;
        a = mktime(&tm);
    }
    printf("%d/%d/%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return 0;
}
