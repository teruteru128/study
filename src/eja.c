
#define _GNU_SOURCE
#include "eja.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>
#include <stdint.h>

/**
 * @brief ejaculation
 *
 */
void eja(int ar) {
  /*
   * y=x/(1+x)
   * y(1+x)=x
   * y + xy = x
   * y=x-xy
   * y=x(1-y)
   * x=y/(1-y)
   * x=y/(4-y)
   * x(4-y) = y
   * 4x -xy = y
   * y + xy = 4x
   * y(1+x)=4x
   * y=4x/(1+x)
   * パラメータ一覧
   * - 棒とか玉のサイズ
   * - 快感の強さ
   * - 射精量スケール
   * - 安全リミット
   */
  const double minp = log10(2);
  const double maxp = log10(10000);
  const double pwindow = maxp - minp;
  double exp = 0;
  int exp1;
  double body;
  uint64_t buffer;
  switch (ar) {
  case 0:
    for (double i = 0; i <= 1000; i++) {
      exp = fmin(6 * (i / 100.0) / (2 + (i / 100.0)), maxp);
      printf("p : %f, %fL\n", exp, pow(10, exp) / 1000);
    }
    break;
  case 1:
    for(int i = 0; i < 1000; i++){
        getrandom(&buffer, sizeof(uint64_t), 0);
        exp1 = -__builtin_ctzll(~(buffer >> 52));
        buffer &= 0xfffffffffffffULL;
        buffer = ((exp1+1022LL) << 52) | buffer;
        memcpy(&exp, &buffer, sizeof(uint64_t));
        body = exp10(pwindow * exp + minp);
        printf("%lf cm(%lf m)\n", body, body / 100.);
    }
    break;
  default:
    fprintf(stderr, "oo\n");
    break;
  }
}
