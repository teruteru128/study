
#include <stdio.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    struct tm tm = { 0 };
    tm.tm_year = 2022 - 1900;
    tm.tm_mon = 1;
    tm.tm_mday = 13;
    time_t a = 0;
    for (int count = 0; count < 3; count++)
    {
        a = mktime(&tm);
        while (localtime_r(&a, &tm)->tm_wday != 5)
        {
            tm.tm_mon++;
            if (tm.tm_mon >= 12)
            {
                tm.tm_mon = 0;
                tm.tm_year++;
            }
            a = mktime(&tm);
        }
        printf("%d/%d/%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        tm.tm_mon++;
        if (tm.tm_mon >= 12)
        {
            tm.tm_mon = 0;
            tm.tm_year++;
        }
    }

    return 0;
}
