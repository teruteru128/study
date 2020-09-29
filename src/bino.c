
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#define X 25

/**
 * binomial coefficient
 * 二項係数プログラム
 * binomial coefficient
 *
 * bino
 * 二項係数のピラミッドが1から25まで並ぶ
 *
 * bino 13
 * 二項係数の13段目のみ列挙
 *
 * bino 13 6
 * 二項係数の13の6のみ
 */
int main(int argc, char *argv[])
{
  long array1[X], array2[X];
  int i,j;
  /*
  for(i = 0; i < X; i++)
  {
    array1[i] = array2[i] = 0;
  }
  */
  for(i = 0; i < X; i++)
    array1[i] = 0;
  array1[0] = 1;
  for(i = 0; i < X; i++)
  {
    for(j = 0; j <= i; j++)
    {
      array2[j] = array1[j];
    }
    for(j = 1; j <= i; j++)
    {
      array2[j] += array1[j - 1];
    }
    for(j = 0; j <= i; j++)
    {
      printf("%ld%s", array2[j], j != i ? " " : "\n");
      array1[j] = array2[j];
    }
  }
  return EXIT_SUCCESS;
}
