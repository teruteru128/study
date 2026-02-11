
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define CNT_MAX 50

int main(const int argc, const char *argv[])
{
    //if (argc < 2)
    //{
    //    return 1;
    //}
    setlocale(LC_ALL, ""); // 必要
    char work[] = "やったぜ。\U00029e49\U0001F4A9";
    const char *p = (argc >= 2) ? argv[1] : work;
    wchar_t out[BUFSIZ] = L"";
    size_t length = 0;
    char tmpc[CNT_MAX];
    const char *tmpd = tmpc;
    wchar_t tmpw[CNT_MAX] = L"";
    mbstate_t mbs1 = {0};
    mbstate_t mbs2 = {0};
    size_t len = 0;
    while (*p != '\0')
    {
        // 次のマルチバイト文字のバイト数を決定する
        len = mbrlen(p, CNT_MAX, &mbs1);
        // (size_t)-1もほぼありえないとはいえ実際は有効な値の範囲なんだよなあ……
        if (len == (size_t)-1 || len == (size_t)-2)
        {
            perror("mbrlen");
            return 1;
        }
        strncpy(tmpc, p, len);
        tmpc[len] = '\0';
        // マルチバイト文字列をワイド文字列に変換する（再開可能）
        size_t wl = mbsrtowcs(tmpw, &tmpd, CNT_MAX, &mbs2);
        wcscat(out + length, tmpw);
        p += len;
        length += wl;
        tmpd = tmpc;
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
