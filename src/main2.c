
#define _GNU_SOURCE
#include "config.h"
#include <bm.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    unsigned char ripe[20] = "";
    memset(ripe, 0xff, 20);

    char *address = NULL;
    for (size_t i = 0; i < 20; i++)
    {
        ripe[i] = 0;
        address = encodeV4Address(ripe, 20);

        printf("%zu, %s\n", strlen(address), address);

        free(address);
    }

    return 0;
}
