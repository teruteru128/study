
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <byteswap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

#define ROLLS 28
#define DICE_SIZE 5

// 暗号論的に安全な乱数を使ったサイコロのソフトウェア実装
int main(int argc, char const *argv[])
{
#if 0
    int64_t seed = 0;
    getrandom(&seed, sizeof(int64_t), GRND_NONBLOCK);
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
#endif
    size_t dice0;
    size_t dice1;
    size_t dice2;
    size_t dice3;
    size_t dice4;
    size_t dice5;
    size_t dice6;
    size_t dice7;
    size_t sumofroll[41] = { 0 };
    for (dice0 = 0; dice0 < 6; dice0++)
    {
        for (dice1 = 0; dice1 < 6; dice1++)
        {
            for (dice2 = 0; dice2 < 6; dice2++)
            {
                for (dice3 = 0; dice3 < 6; dice3++)
                {
                    for (dice4 = 0; dice4 < 6; dice4++)
                    {
                        for (dice5 = 0; dice5 < 6; dice5++)
                        {
                            for (dice6 = 0; dice6 < 6; dice6++)
                            {
                                for (dice7 = 0; dice7 < 6; dice7++)
                                {
                                    sumofroll[dice0 + dice1 + dice2 + dice3
                                              + dice4 + dice5 + dice6
                                              + dice7]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    double r = 0;
    double sum = 0;
    for (size_t rollnum = 0; rollnum < 41; rollnum++)
    {
        r = (sumofroll[rollnum]) / 1679616.;
        sum += r;
        printf("%2zu: %7zu, %lf%%, %lf%%, %d\n", rollnum + 8, sumofroll[rollnum],
               r * 100, sum * 100, sum < (1 / 1000000.));
    }
    return EXIT_SUCCESS;
}
