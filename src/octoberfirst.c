
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    time_t currenttime = 0;
    struct tm c_tm = { 0 };
    struct tm target_tm = { 0 };
    time_t target_time = 0;
    int64_t allofseconds = 0;
    ldiv_t dayandsec = { 0 };
    ldiv_t hourandsec = { 0 };
    ldiv_t minandsec = { 0 };
    struct timespec request = { 0 };
    // 1秒に20回
    request.tv_sec = 0;
    request.tv_nsec = 50000000L;
    while (1)
    {
        // 現在時刻取得
        currenttime = time(NULL);
        localtime_r(&currenttime, &c_tm);
        target_tm.tm_year = c_tm.tm_year;
        target_tm.tm_mon = 9;
        target_tm.tm_mday = 1;
        // 今年の10月1日午前0時0分0秒の時刻を取得
        target_time = mktime(&target_tm);
        // 差分を取る
        allofseconds = (int64_t)difftime(target_time, currenttime);
        if (allofseconds <= 0)
        {
            // 現在時刻は10月1日午前0時0分0秒以降
            // 来年の10月1日
            target_tm.tm_year++;
            target_time = mktime(&target_tm);
            allofseconds = difftime(target_time, currenttime);
        }
        dayandsec = ldiv(allofseconds, 86400);
        hourandsec = ldiv(dayandsec.rem, 3600);
        minandsec = ldiv(hourandsec.rem, 60);
        fprintf(stdout,
                "次の10月1日まであと%" PRId64 "日%" PRId64 "時間%" PRId64
                "分%" PRId64 "秒\r",
                dayandsec.quot, hourandsec.quot, minandsec.quot,
                minandsec.rem);
        fflush(stdout);
        nanosleep(&request, NULL);
    }

    return 0;
}
