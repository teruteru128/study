
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 2038年問題カウントダウン
int countdown2038(void)
{
    time_t u = 0x80000000L;
    ldiv_t quot = { 0 };
    for (long diff = (long)difftime(u, time(NULL)); diff > 0; diff = (long)difftime(u, time(NULL)))
    {
        quot = ldiv(diff, 86400L);
        printf("2038年1月19日12時14分8秒まであと%ld日%ld秒\n", quot.quot,
               quot.rem);
        sleep(1);
    }
    return 0;
}
