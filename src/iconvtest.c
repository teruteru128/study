
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <iconv.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SRC                                                                        \
    "551 5 ABCDEFG:2005/03/27 "                                                    \
    "12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6," \
    "仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

int main(argc, argv) int argc;
char **argv;
{
    setlocale(LC_ALL, "");
    printf("%lu\r\n", strlen(SRC));
    iconv_t cd = iconv_open("Shift-JIS", "UTF-8");
    if (cd == (iconv_t)-1)
    {
        perror("iconv_open");
        return EXIT_FAILURE;
    }
    size_t srclen = strlen(SRC);
    char *tmpsrc = strdup(SRC);
    // iconv を呼び出すと tmpsrc が指すポインタが書き換わるため
    // free用にポインタをバックアップして置かなければならない
    char *srcwork = tmpsrc;

    size_t destlen = srclen * 3;
    char *destbuf = malloc(destlen + 1);
    if (!destbuf)
    {
        perror("malloc");
        return EXIT_FAILURE;
    }
    char *head = destbuf;
    errno = 0;
    size_t ret = iconv(cd, &srcwork, &srclen, &destbuf, &destlen);
    if (ret == (size_t)-1)
    {
        perror("iconv");
        return -1;
    }
    if (ret != 0)
    {
        printf("%zu文字が非可逆変換されました。\n", ret);
    }
    *destbuf = '\0';

    free(tmpsrc);

    char *tmp = realloc(head, strlen(head) + 1);
    if (tmp != NULL)
    {
        head = tmp;
    }
    printf("%lu\r\n", strlen(head));
    printf("%s", head);
    free(head);
    int r = iconv_close(cd);
    if (r != 0)
    {
        perror("iconv_close");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
