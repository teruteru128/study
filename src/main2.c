
#define _GNU_SOURCE
#include "config.h"

#include <stddef.h>
#include <stdio.h>

#include <bm.h>
#include <err.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");

    const char *txt = argv[1];

    char buf[BUFSIZ];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    // XX日を出せるやつはなさそうですね……
    strftime(buf, BUFSIZ, "%FT%T%z/%Ex/%EX/%z/%b/%B%n%Ey/%Ec/%EC/%d/\"%D\"/%c%n%C/%X/%EX/%p %a %A", &tm);
    printf("%s\n", buf);
    return 0;
}
