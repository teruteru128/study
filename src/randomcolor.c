
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *random = fopen("/dev/random", "rb");
    if (random == NULL)
    {
        perror("fopen random");
        return EXIT_FAILURE;
    }
    unsigned char buf[18];
    size_t num = fread(buf, 1, 18, random);
    if (num == 0)
    {
        return EXIT_FAILURE;
    }
    fclose(random);
    unsigned int j = 0;
    unsigned int k = 0;
    for (size_t i = 0; i < 6; i++)
    {
        memcpy(&j, &buf[i], 3);
        k = (buf[i * 3] << 16U) | (buf[i * 3 + 1] << 8U) | (buf[i * 3 + 2] << 0U);
        printf("#%02x%02x%02x, %06x, %06x, %06x\n", buf[i * 3], buf[i * 3 + 1], buf[i * 3 + 2], htole32(j), htobe32(j), k);
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
