
#define _GNU_SOURCE
#include "ochinchin.h"
#include "eja.h"
#include "ftnr.h"
#include "penis.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/random.h>

void eja(int ar);

int main(int argc, char const *argv[])
{
    /*
    size_t length = 27;
    for (size_t i = 1; length <= 300; i++)
    {
        printf("第%ld形態: %zucm\n", i, length);
        penis(length);
        length += i * 3;
    }
    */
    if (argc < 2)
    {
        return EXIT_SUCCESS;
    }
    if(strcmp(argv[1], "ftnr") == 0)
    {
        if(argc < 3)
        {
            fprintf(stderr, "引数不足\n");
            return 1;
        }
        uint64_t a = strtoull(argv[2], NULL, 10);
        ftnr_penis((int)a);
        return 0;
    }
    else if(strcmp(argv[1], "eja") == 0)
    {
        eja(argc < 3?0:atoi(argv[2]));
        return 0;
    }
    else if(strcmp(argv[1], "penis") == 0)
    {
        if(argc < 3)
        {
            fprintf(stderr, "引数不足\n");
            return 1;
        }
        penis(strtoull(argv[2], NULL, 10));
        return 0;
    }
}

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
