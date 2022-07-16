
#define _GNU_SOURCE
#include "config.h"

#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <sys/random.h>

int main(int argc, char const *argv[])
{
    unsigned char buf[96];

    for (size_t i = 0; i < 96; i++)
    {
        printf("%02x", buf[i]);
    }
    fputs("\n", stdout);

    ssize_t len = getrandom(buf, 96, 0);
    if (len < 0)
    {
        return 1;
    }
    for (size_t i = 0; i < 96; i++)
    {
        printf("%02x", buf[i]);
    }
    fputs("\n", stdout);
    len = getrandom(buf, 96, GRND_NONBLOCK);
    if (len < 0)
    {
        return 1;
    }
    for (size_t i = 0; i < 96; i++)
    {
        printf("%02x", buf[i]);
    }
    fputs("\n", stdout);
    len = getrandom(buf, 96, GRND_RANDOM);
    if (len < 0)
    {
        return 1;
    }
    for (size_t i = 0; i < 96; i++)
    {
        printf("%02x", buf[i]);
    }
    fputs("\n", stdout);
    len = getrandom(buf, 96, GRND_INSECURE);
    if (len < 0)
    {
        return 1;
    }
    for (size_t i = 0; i < 96; i++)
    {
        printf("%02x", buf[i]);
    }
    fputs("\n", stdout);
    return 0;
}
