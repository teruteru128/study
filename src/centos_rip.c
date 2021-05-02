
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

/**
 * @brief see https://centos.rip/
 * 
 */
void CentOS_rip()
{
    struct tm countDownTM;
    countDownTM.tm_sec = 0;
    countDownTM.tm_min = 0;
    countDownTM.tm_hour = 0;
    countDownTM.tm_mday = 31;
    countDownTM.tm_mon = 11;
    countDownTM.tm_year = 2021 - 1900;
    countDownTM.tm_wday = 0;
    countDownTM.tm_yday = 0;
    countDownTM.tm_isdst = 0;
#ifdef __USE_MISC
    countDownTM.tm_gmtoff = 0;
    countDownTM.tm_zone = NULL;
#else
    countDownTM.__tm_gmtoff = 0;
    countDownTM.__tm_zone = NULL;
#endif
    const time_t countDownDate = mktime(&countDownTM);
    struct timespec spec;
    spec.tv_sec = 0;
    // 1/10
    // 0.010000000
    //    10000000
    // 1/20
    // 0.005000000
    //     5000000
    // 1/128
    // 0.007812500
    //     7812500
    spec.tv_nsec = 5000000L;

    struct timespec nowspec;
    long secs = 0;
    long days;
    long hours;
    long minutes;
    long seconds;
    long milliseconds;
    while (true)
    {
        clock_gettime(CLOCK_REALTIME, &nowspec);
        secs = (long)difftime(countDownDate, nowspec.tv_sec);
        if (secs < 0)
        {
            break;
        }
        days = secs / (60 * 60 * 24);
        hours = (secs % (60 * 60 * 24)) / (60 * 60);
        minutes = (secs % (60 * 60)) / 60;
        seconds = secs % 60;
        milliseconds = nowspec.tv_nsec / 1000000;
        fprintf(stdout, "%03ldd%02ldh%02ldm%02lds%02ld\r", days, hours, minutes, seconds, milliseconds);
        fflush(stdout);
        nanosleep(&spec, NULL);
    }
}
