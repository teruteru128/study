
#include "yattaze.h"
#include <stdio.h>
#include <string.h>

void printYattaze(void) {
    // TODO: ヘッダーファイルに本文を埋め込むべきか、ファイルに書き込まれたファイルを読み込むべきか？
    fputs(YATTAZE, stdout);
}

int main(void)
{
    printYattaze();
    return 0;
}
