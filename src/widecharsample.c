
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define CNT_MAX 50

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, ""); //必要
    char work[] = "やったぜ。\U00029e49";
    const char *p = work;
    wchar_t out[BUFSIZ] = L"";
    size_t length = 0;
    char tmpc[CNT_MAX];
    const char *tmpd = tmpc;
    wchar_t tmpw[CNT_MAX] = L"";
    mbstate_t mbs1 = { 0 };
    mbstate_t mbs2 = { 0 };
    while (*p != '\0')
    {
        tmpd = tmpc;
        size_t l = mbrlen(p, CNT_MAX, &mbs1);
        // (size_t)-1もほぼありえないとはいえ実際は有効な値の範囲なんだよなあ……
        if (l == (size_t)-1 || l == (size_t)-2)
        {
            perror("mbrlen");
            return 1;
        }
        strncpy(tmpc, p, l);
        tmpc[l] = '\0';
        size_t wl = mbsrtowcs(tmpw, &tmpd, CNT_MAX, &mbs2);
        wcscat(out + length, tmpw);
        p += l;
        length += wl;
    }
    printf("%ls\n", out);
    for (size_t i = 0; i < length; i++)
    {
        printf("\\U%08x\n", out[i]);
    }
    // wchar_t ->
    // char[]に変換するときはwcstombs(NULL,src,0)+1のnにしなければならない

    return 0;
}
