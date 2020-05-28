
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 5
#define MINZ (-313)
#define MAXZ 312
#define MINX (-313)
#define MAXX 312

/* 移動合計 */
void moving_sum()
{
}
void moving_average()
{
}

int maxmovingsum(int *array, const size_t datasize, const size_t windowsize)
{
}

int main(int argc, char *argv[])
{
  int rawchunk[625];
  int movingsum[621];
  int maxtmp = 0;
  srand(114514);
  int i = 0;
  for (i = 0; i < 625; i++)
  {
    rawchunk[i] = (rand() % 10) == 0;
  }
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

  //
  return EXIT_SUCCESS;
}
