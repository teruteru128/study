
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <limits.h>
#include <printint.h>

#define BUF_SIZE 21
#define LOOP_COUNT 100000000ULL

/**
 * printint benchmark
 * */
int main(int argc, char **argv)
{
    char buf[BUF_SIZE];
    uint64_t t = 0;
    size_t i;
    size_t len;
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &start);
    for (i = 0; i < LOOP_COUNT; i++)
    {
        len = snprintUInt64(buf, BUF_SIZE, t);
        //snprintf(buf, BUF_SIZE, "%" PRIu64, t);
        //len = strlen(buf);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    time_t sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    double passed = (sec * 1e9) + nsec;
    double seconds = passed / 1e9;
    fprintf(stderr, _("It took %g seconds.\n"), seconds);
    fprintf(stderr, _("%g times per second\n"), LOOP_COUNT / seconds);
    fprintf(stderr, _("%e seconds per time\n"), seconds / LOOP_COUNT);
    printf("%" PRIu64 "\n", len);
}
