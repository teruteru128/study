
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <java_random.h>

#define WIDTH 5
#define MINZ (-313)
#define MAXZ 312
#define MINX (-313)
#define MAXX 312

/**
 * 移動合計
 * 出力配列のチェックとかめんどくさいんですけどねー
 * 出力配列は呼び出し側で確保する？(int *outarray, size_t outarraysize)
 * 呼び出された側？(int **outarray, size_t *outarraysize)もしくは返り値でint *を返す？
 */
int moving_sum(int *inarray, const size_t datasize, const size_t windowsize, int *outarray, size_t outarraysize)
{
  if(inarray == NULL || datasize == 0 || windowsize == 0 || outarray == NULL || outarraysize == 0)
  {
    return 1;
  }
  int sum = 0;
  for(size_t i = 0; i < windowsize; i++)
  {
    sum += inarray[i];
  }
  outarray[0] = sum;
  for(size_t i = windowsize; i < datasize; i++)
  {
    sum -= inarray[i - windowsize];
    sum += inarray[i];
    outarray[i - windowsize + 1] = sum;
  }
  return 0;
}

/* 移動平均 */
void moving_average(int *array, const size_t datasize, const size_t windowsize)
{
}

/**
 * 移動合計の最大値
 * 移動合計と配列の最大値の合成にすべきでは？
 * この関数はintで実装したけど、変数サイズと符号あり/なしで全部バラバラの実装するの？やだなあ……
 */
int moving_sum_max(int *inarray, const size_t datasize, const size_t windowsize)
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
  int maxtmp = 0;
  srand(114514);
  int64_t seed = n(114514);
  for (int i = 0; i < 625; i++)
  {
    rawchunk[i] = nextIntWithBounds(&seed, 10) == 0;
    printf("%d", rawchunk[i]);
  }
  printf("\n");
  int max = moving_sum_max(rawchunk, 625, 5);
  printf("%d\n", max);
  /*
  // https://www.ei.fukui-nct.ac.jp/2018/06/05/moving-average-program/
  int bp = 0;
  //const int winsize=5;
  int buf[WIDTH];
  int sum = 0;
  int widz = MAXZ - MINZ;
  int z;
  int widx = MAXX - MINX;
  int x;
  for (z = MINZ; z < MAXZ; z++)
  {
    i = (z + widz) % WIDTH;
    sum = sum - buf[i];
    buf[i] = (rand() % 10) == 0;
  }
  fputs("\n", stdout);
  */

  //
  return EXIT_SUCCESS;
}
