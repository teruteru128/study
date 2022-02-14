
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
    countDownTM.tm_sec = 0;
    countDownTM.tm_min = 0;
    countDownTM.tm_hour = 0;
    countDownTM.tm_mday = 31;
    countDownTM.tm_mon = 11;
    countDownTM.tm_year = 2021 - 1900;
    countDownTM.tm_wday = 0;
    countDownTM.tm_yday = 0;
    countDownTM.tm_isdst = 0;
    const time_t countDownDate = mktime(&countDownTM);
    const time_t now = time(NULL);

    if (difftime(countDownDate, now) <= 0)
    {
        fprintf(stdout, "もう終わっとるわ\n");
        return 1;
    }

    long secs = 0;
    long days;
    long hours;
    long minutes;
    long seconds;
    long milliseconds;
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
    secs = (long)difftime(countDownDate, nowspec.tv_sec);
    ssize_t c = 0;
    int ret = 0;
    do
    {
        days = secs / (60 * 60 * 24);
        hours = (secs % (60 * 60 * 24)) / (60 * 60);
        minutes = (secs % (60 * 60)) / 60;
        seconds = secs % 60;
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
        secs = (long)difftime(countDownDate, nowspec.tv_sec);
    } while (secs > 0);
    if (c == -1)
    {
        ret = 1;
        perror("read");
    }
    close(timerfd);
    return ret;
}
