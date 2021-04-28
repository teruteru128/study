
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{

    char aaa[8] = "abcdefg";
    char bbb[4][8] = {"adhoc", "bluetoo", "compact", "digital"};
    // (char *) への配列
    char *test1[8] = {&aaa[1], &aaa[2], NULL};
    printf("%s\n", test1[0]);
    printf("%s\n", test1[1]);
    // char[8] へのポインタ
    char(*test2)[8] = &aaa;
    printf("%s\n", test2[0]);
    test2 = bbb;
    printf("%s\n", test2[1]);
    return 0;
}
