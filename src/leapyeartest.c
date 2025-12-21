
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "timeutil.h"
#include <locale.h>

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
    strftime(buf, BUFSIZ, "%FT%T%z", &uruu2);
    printf("%s, %d\n", buf, tmcomp(&uruu1, &uruu2));
    // setlocale(3) を呼び出すかどうかで挙動が変わる
    strftime(buf, BUFSIZ, "%Ex %EX %z", &uruu2);
    printf("%s\n", buf);
    t = time(NULL);
    localtime_r(&t, &uruu2);
    strftime(buf, BUFSIZ, "%EC/%Ey/%Ec/%X/%EX/%p", &uruu2);
    printf("%s\n", buf);
}

int main(int argc, char const *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "--setlocale") == 0)
    {
        setlocale(LC_ALL, "");
    }
    leapyeartest();
    return 0;
}
