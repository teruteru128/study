
#define _GNU_SOURCE
#include "config.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

    char buf[BUFSIZ] = "";
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    // XX日を出せるやつはなさそうですね……
    size_t len = strftime(buf, BUFSIZ, "%FT%T%z/%Ex/%EX/%z/%b/%B%n%Ey/%Ec/%EC/%d/\"%D\"/%c%n%C/%X/%EX/%p %a %A", &tm);
    printf("%zu : %s\n", len, buf);
    char msg[] = "\xe2\x80\xa8\xe2\x80\xa8";
    printf("%s\n", msg);
    printf("%02x%02x%02x\n", msg[0] & 0xff, msg[1] & 0xff, msg[2] & 0xff);
    wchar_t wc[2] = { 0, 32 };
    // 1文字分だけ変換
    int ret = mbtowc(wc, msg, 6);
    printf("%d : %x %x\n", ret, wc[0], wc[1]);
    return 0;
}
