
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "java_random.h"
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <errno.h>

/*
 * 9.3cm, 6.243646ml
 * 10.0cm, 47081.586164ml
 * 11.9cm, 304.445320ml
 * 18.2cm, 8.055883ml
 * 20.3cm, 4.031057ml
 * 21.1cm, 4608.490765ml
 * 25.2cm, 582.253892ml
 * 27.7cm, 2.804713ml
 * 28.2cm, 10962.108695ml
 * 29.7cm, 27210.034929ml
 * 31.8cm, 2037.117076ml
 * 32.9cm, 5.508962ml
 * 36.5cm, 1698.530900ml
 * 38.2cm, 74641.376689ml
 * 38.4cm, 167.290089ml
 * 39.4cm, 3465.697319ml
 *
 * 27.7cm, 2.804713ml
 * 20.3cm, 4.031057ml
 * 32.9cm, 5.508962ml
 * 9.3cm, 6.243646ml
 * 18.2cm, 8.055883ml
 * 38.4cm, 167.290089ml
 * 11.9cm, 304.445320ml
 * 25.2cm, 582.253892ml
 * 36.5cm, 1698.530900ml
 * 31.8cm, 2037.117076ml
 * 39.4cm, 3465.697319ml
 * 21.1cm, 4608.490765ml
 * 28.2cm, 10962.108695ml
 * 29.7cm, 27210.034929ml
 * 10.0cm, 47081.586164ml
 * 38.2cm, 74641.376689ml
 */
int ftnr_penis(int argc, char const *argv[])
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int64_t seed = ((ts.tv_nsec & 0x00FFFFFFL) << 24) + ts.tv_sec;
    char statebuf[BUFSIZ];
    struct random_data buf;
    errno = 0;
    initstate_r(seed, statebuf, BUFSIZ, &buf);
    if (srandom_r(seed, &buf) != 0)
    {
        perror("srandom_r");
        return EXIT_FAILURE;
    }
    int32_t dick1;
    int32_t dick2;
    for (size_t i = 0; i < 5; i++)
    {
        if (random_r(&buf, &dick1))
        {
            perror("random_r 1");
            return EXIT_FAILURE;
        }
        if (random_r(&buf, &dick2))
        {
            perror("random_r 2");
            return EXIT_FAILURE;
        }
        printf("%.1fcm, %lfml\n", ((double)dick1 / RAND_MAX * 310 + 90) / 10, pow(10, (double)dick2 / RAND_MAX * 4));
    }
    return EXIT_SUCCESS;
}
