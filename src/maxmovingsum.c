
#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <java_random.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

#define WIDTH 5
#define MINZ (-313)
#define MAXZ 312
#define MINX (-313)
#define MAXX 312

/**
 * 移動合計
 * 出力配列のチェックとかめんどくさいんですけどねー
 * 出力配列は呼び出し側で確保する？(int *outarray, size_t outarraysize)
 * 呼び出された側？(int **outarray, size_t *outarraysize)もしくは返り値でint
 * *を返す？
 */
int moving_sum(const int *inarray, const size_t datasize,
               const size_t windowsize, int *outarray, size_t outarraysize)
{
    if (inarray == NULL || datasize == 0 || windowsize == 0 || outarray == NULL
        || outarraysize == 0)
    {
        return 1;
    }
    int sum = 0;
    for (size_t i = 0; i < windowsize; i++)
    {
        sum += inarray[i];
    }
    outarray[0] = sum;
    for (size_t i = windowsize; i < datasize; i++)
    {
        sum -= inarray[i - windowsize];
        sum += inarray[i];
        outarray[i - windowsize + 1] = sum;
    }
    return 0;
}

/* 移動平均 */
void moving_average(int *inarray, const size_t datasize,
                    const size_t windowsize, float *outarray,
                    size_t outarraysize)
{
    if (inarray == NULL || datasize == 0 || windowsize == 0 || outarray == NULL
        || outarraysize == 0)
    {
        return;
    }
    int sum = 0;
    for (size_t i = 0; i < windowsize; i++)
    {
        sum += inarray[i];
    }
    outarray[0] = (float)sum / (float)windowsize;
    for (size_t i = windowsize; i < datasize; i++)
    {
        sum -= inarray[i - windowsize];
        sum += inarray[i];
        outarray[i - windowsize + 1] = (float)sum / (float)windowsize;
    }
    return;
}

/**
 * 移動合計の最大値
 * 移動合計と配列の最大値の合成にすべきでは？
 * この関数はintで実装したけど、変数サイズと符号あり/なしで全部バラバラの実装するの？やだなあ……
 */
int moving_sum_max(const int *inarray, const size_t datasize,
                   const size_t windowsize)
{
    int sum = 0;
    for (size_t i = 0; i < windowsize; i++)
    {
        sum += inarray[i];
    }
    int max = INT_MIN;
    for (size_t i = windowsize; i < datasize; i++)
    {
        sum -= inarray[i - windowsize];
        sum += inarray[i];
        if (max < sum)
        {
            max = sum;
        }
    }
    return max;
}

int main(int argc, char *argv[])
{
    int rawchunk[625];
    int movingsum[621];
    float movingsum_f[621];
    int maxtmp = 0;
    unsigned int buf = 0;
    ssize_t numberOfRandomBytes
        = getrandom(&buf, sizeof(unsigned int), GRND_NONBLOCK);
    if (numberOfRandomBytes < 0)
    {
        return 1;
    }
    srand(buf);
    struct drand48_data seed = { 0 };
    int64_t l = initialScramble(lcg(114514));
    unsigned short a[3];
    a[0] = (unsigned short)l;
    a[1] = (unsigned short)(l >> 16);
    a[2] = (unsigned short)(l >> 32);
    seed48_r(a, &seed);
    for (int i = 0; i < 625; i++)
    {
        rawchunk[i] = nextIntWithBounds(&seed, 10) == 0;
        printf("%d", rawchunk[i]);
    }
    printf("\n");
    int max = moving_sum_max(rawchunk, 625, 5);
    printf("%d\n", max);
    moving_sum(rawchunk, 625, 5, movingsum, 621);
    moving_average(rawchunk, 625, 5, movingsum_f, 621);
    // https://www.ei.fukui-nct.ac.jp/2018/06/05/moving-average-program/

    //
    return EXIT_SUCCESS;
}
