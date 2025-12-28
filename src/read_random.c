
#include <base64.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <arpa/inet.h>
#if 0
#define D_SIZE (786432)
#else
#define D_SIZE (64)
#endif
/**
  TODO: 出力フォーマット, データ長
  32bit 符号付き/符号なし整数
  64bit 符号付き/符号なし整数
  0.0以上1.0未満の32bit/64bit浮動小数点
  base64
  生バイナリ
*/
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    size_t length1 = strtoull(argv[1], NULL, 10);

    uint8_t *buf1 = malloc(length1);
    if (buf1 == NULL)
    {
        return EXIT_FAILURE;
    }
    size_t total = 0;
    ssize_t numberOfRandomBytes;
    while (total < length1)
    {
        numberOfRandomBytes = getrandom(buf1 + total, length1 - total, GRND_NONBLOCK);
        total += numberOfRandomBytes;
        fprintf(stderr, "%zd bytes readed\n", numberOfRandomBytes);
    }

    char *base64 = base64encode(buf1, length1);
    free(buf1);
    buf1 = NULL;
    size_t length = strlen(base64);
    size_t unit = length / 4;
    fprintf(stderr, "readed : %u, length : %lu, unit : %lu\n", BUFSIZ, length,
            unit);
    printf("%s\n", base64);
    free(base64);
    return EXIT_SUCCESS;
}
