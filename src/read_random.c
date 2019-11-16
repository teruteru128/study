
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
#if 0
#define D_SIZE (786432)
#else
#define D_SIZE (8)
#endif

size_t read_random(void *buf, size_t len, int use_true_random){
  char *path[]={
  "/dev/random",
  "/dev/urandom"
  };
  printf("%s\n", path[!use_true_random]);
  return 0;
}
/*
  TODO: 出力フォーマット, データ長
  32bit 符号付き/符号なし整数
  64bit 符号付き/符号なし整数
  0.0以上1.0未満の32bit/64bit浮動小数点
  base64
  生バイナリ
*/
int main(int argc, char* argv[]){
  char* path = "/dev/urandom";
  FILE* fp = NULL;
  read_random(NULL, 0, 0);
  read_random(NULL, 0, 1);

  int rc = EXIT_SUCCESS;
  fp = fopen(path, "rb");
  if(fp == NULL){
    perror("fopen");
    return EXIT_FAILURE;
  }

  size_t len = 0;
  char *buf1 = NULL;
  buf1 = calloc(D_SIZE, sizeof(char));
  if((len = fread(buf1, sizeof(char), D_SIZE, fp)) < D_SIZE){
    perror("fread");
    rc = EXIT_FAILURE;
    goto end;
  }

  if(fclose(fp)){
    perror("fclose");
    rc = EXIT_FAILURE;
    goto end;
  }
  char* base64 = base64encode(buf1, len);
  size_t length = strlen(base64);
  size_t unit = length / 4;
  fprintf(stderr, "readed : %u, length : %lu, unit : %lu\n", D_SIZE, length, unit);
  printf("%s\n", base64);
end:
  free(buf1);
  buf1 = NULL;
  return EXIT_SUCCESS;
}
