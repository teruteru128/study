
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

int main(int argc, char *argv[])
{
    unsigned char buf[18];
    ssize_t numberOfRandomBytes = getrandom(buf, 18, GRND_RANDOM);
    if (numberOfRandomBytes < 0)
    {
        return 1;
    }
    unsigned int j = 0;
    unsigned int k = 0;
    for (size_t i = 0; i < 6; i++)
    {
        memcpy(&j, &buf[i], 3);
        k = (buf[i * 3] << 16U) | (buf[i * 3 + 1] << 8U)
            | (buf[i * 3 + 2] << 0U);
        printf("#%02x%02x%02x, %06x, %06x, %06x\n", buf[i * 3], buf[i * 3 + 1],
               buf[i * 3 + 2], htole32(j), htobe32(j), k);
    }
    for (size_t i = 0; i < 18; i++)
    {
        if (i % 3 == 0)
            fputs("#", stdout);
        printf("%02x", buf[i]);
        if (i % 3 == 2)
            fputs("\n", stdout);
    }
    return EXIT_SUCCESS;
}
