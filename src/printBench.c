
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <limits.h>
#include <printint.h>
#include <math.h>
#include "timeutil.h"

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
    struct timespec diff;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < LOOP_COUNT; i++)
    {
        len = snprintUInt64(buf, BUF_SIZE, t);
        //snprintf(buf, BUF_SIZE, "%" PRIu64, t);
        //len = strlen(buf);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    difftimespec(&diff, &end, &start);
    double seconds = fma((double)diff.tv_sec, 1e9, (double)diff.tv_nsec) / 1e9;
    fprintf(stderr, _("It took %g seconds.\n"), seconds);
    fprintf(stderr, _("%g times per second\n"), LOOP_COUNT / seconds);
    fprintf(stderr, _("%e seconds per time\n"), seconds / LOOP_COUNT);
    printf("%" PRIu64 "\n", len);
}
