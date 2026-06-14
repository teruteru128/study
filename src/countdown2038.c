
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 2038年問題カウントダウン
int countdown2038(void)
{
    time_t u = 0x80000000L;
    ldiv_t quot = {0};
    struct timespec s;
    s.tv_sec = 0;
    s.tv_nsec = 800 * 1000 * 1000;
    for (long diff = (long)difftime(u, time(NULL)); diff > 0; diff = (long)difftime(u, time(NULL)))
    {
        quot = ldiv(diff, 86400L);
        printf("2038年1月19日12時14分8秒まであと%ld日%ld秒\n", quot.quot,
               quot.rem);
        nanosleep(&s, NULL);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    return countdown2038();
}
