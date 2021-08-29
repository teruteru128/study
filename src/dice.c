
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>

#define ROLLS 28
#define DICE_SIZE 5

// 暗号論的に安全な乱数を使ったサイコロのソフトウェア実装
int main(int argc, char const *argv[])
{
    FILE *in = fopen("/dev/urandom", "rb");
    if (!in)
    {
        perror("fopen urandom");
        return EXIT_FAILURE;
    }
    int64_t seed = 0;
    size_t num = fread(&seed, sizeof(char), 8, in);
    fclose(in);
    if (num != 8)
    {
        perror("fread");
        return EXIT_FAILURE;
    }
    seed = htole64(seed);
    srandom(((seed >> 32) & 0xffffffff) ^ (seed & 0xffffffff));
    int roll[ROLLS];
    for (size_t i = 0; i < ROLLS; i++)
    {
        roll[i] = ((double)random() / RAND_MAX) * DICE_SIZE + 1;
    }
    /*
     * https://hg.openjdk.java.net/jdk/jdk14/file/6c954123ee8d/src/java.base/share/classes/java/util/StringJoiner.java
     * joinerの構造
     * 要素配列がnullかつemptyValueがnullでないとき
     *   emptyValueを返す
     * プレフィックスとサフィックスの長さの和をaddLenと置く
     * addLenが0と等しいとき
     *   要素を縮小する
     *   要素数が0のときは空文字列を、さもなくば要素配列の0番目の要素を返す
     * 要素配列のこれまでの文字数の合計とaddLenの大きさを持つ文字列配列を作成してcharsとおく
     * charsにプレフィックスを追加する
     * サイズが0以上のとき
     *   charsに0番目の要素を追加する
     *   for(int i = 1; i < size; i++)
     *     charsにデリミタを追加
     *     charsにi番目を追加
     * charsにサフィックスを追加
     * charsを文字列型に変換して返す
     */
    int64_t sum = roll[0];
    printf("(%d", roll[0]);
    for (size_t i = 1; i < ROLLS; i++)
    {
        printf("+%d", roll[i]);
        sum += roll[i];
    }
    printf(") = %ld\n", sum);
    return EXIT_SUCCESS;
}
