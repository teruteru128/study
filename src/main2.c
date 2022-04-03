
#define _GNU_SOURCE
#include "config.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <bm.h>
#include <err.h>
#include <errno.h>
#include <locale.h>
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
