
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    time_t currenttime = time(NULL);
    struct tm c_tm = { 0 };
    struct tm target_tm = { 0 };
    localtime_r(&currenttime, &c_tm);
    target_tm.tm_year = c_tm.tm_year;
    target_tm.tm_mon = 9;
    target_tm.tm_mday = 1;
    time_t target_time = mktime(&target_tm);
    int64_t allofseconds = (int64_t)difftime(target_time, currenttime);
    if (allofseconds <= 0)
    {
        // 現在時刻は10月1日午前0時0分0秒以降
        // 来年の10月1日
        target_tm.tm_year++;
        target_time = mktime(&target_tm);
        allofseconds = difftime(target_time, currenttime);
    }
    int64_t day = allofseconds / 86400;
    int64_t hour = (allofseconds % 86400) / 3600;
    int64_t min = (allofseconds % 3600) / 60;
    int64_t sec = allofseconds % 60;
    printf("次の10月1日まであと%" PRId64 "日%" PRId64 "時間%" PRId64
           "分%" PRId64 "秒\n",
           day, hour, min, sec);

    return 0;
}
