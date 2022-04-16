
#define _GNU_SOURCE
#include "config.h"

#include <bm.h>
#include <byteswap.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    perror("");

    return 0;
}
