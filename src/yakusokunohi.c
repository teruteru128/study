
#include <stdbool.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief see https://centos.rip/
 * 複数のtimerfdをepoll ファイルディスクリプタで監視することも出来るんですねぇ
 */
int yakusokunohi(void)
{
    struct tm countDownTM = { 0 };
    countDownTM.tm_sec = 22;
    countDownTM.tm_min = 14;
    countDownTM.tm_hour = 7;
    countDownTM.tm_mday = 16;
    countDownTM.tm_mon = 7;
    countDownTM.tm_year = 2021 - 1900;
    countDownTM.tm_wday = 0;
    countDownTM.tm_yday = 0;
    countDownTM.tm_isdst = 0;
    const time_t countDownDate = mktime(&countDownTM);

    struct timespec nowspec;
    clock_gettime(CLOCK_REALTIME, &nowspec);

    struct itimerspec timerspec = { 0 };
    timerspec.it_value.tv_sec = nowspec.tv_sec;
    timerspec.it_value.tv_nsec = nowspec.tv_nsec + 50000000;
    if (timerspec.it_value.tv_nsec >= 1000000000)
    {
        timerspec.it_value.tv_sec++;
        timerspec.it_value.tv_nsec -= 1000000000;
    }
    timerspec.it_interval.tv_sec = 0;
    timerspec.it_interval.tv_nsec = 50000000;

    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    if (timerfd < 0)
    {
        perror("timerfd_create");
        return 1;
    }

    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &timerspec, NULL) != 0)
    {
        perror("timerfd_settime");
        close(timerfd);
        return 1;
    }

    unsigned long count = 0;
    long utime = (long)difftime(countDownDate, nowspec.tv_sec);
    ssize_t c = 0;
    int ret = 0;
    long days;
    long hours;
    long minutes;
    long seconds;
    long milliseconds;
    do
    {
        days = utime / 86400;
        hours = (utime % 86400) / 3600;
        minutes = (utime % 3600) / 60;
        seconds = utime % 60;
        milliseconds = nowspec.tv_nsec / 1000000;
        fprintf(stdout, "%03ldd%02ldh%02ldm%02lds%02ld\r", days, hours,
                minutes, seconds, milliseconds);
        fflush(stdout);
        c = read(timerfd, &count, sizeof(unsigned long));
        if (c == -1)
        {
            break;
        }
        clock_gettime(CLOCK_REALTIME, &nowspec);
        utime = (long)difftime(countDownDate, nowspec.tv_sec);
    } while (utime > 0);
    if (c == -1)
    {
        ret = 1;
        perror("read");
    }
    close(timerfd);
    return ret;
}

int main(void)
{
    yakusokunohi();
    return 0;
}
