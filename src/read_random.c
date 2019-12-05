
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
#include "random.h"
#if 0
#define D_SIZE (786432)
#else
#define D_SIZE (8)
#endif
/*
  TODO: 出力フォーマット, データ長
  32bit 符号付き/符号なし整数
  64bit 符号付き/符号なし整数
  0.0以上1.0未満の32bit/64bit浮動小数点
  base64
  生バイナリ
*/
int main(int argc, char* argv[]){
  uint64_t buf1 = 0;
  read_random(&buf1, sizeof(uint64_t), 1, 0);
  //read_random(NULL, 0, 0, 1);

  char* base64 = base64encode((char*)&buf1, sizeof(uint64_t));
  size_t length = strlen(base64);
  size_t unit = length / 4;
  fprintf(stderr, "readed : %u, length : %lu, unit : %lu\n", D_SIZE, length, unit);
  printf("%s\n", base64);
  free(base64);
  return EXIT_SUCCESS;
}
