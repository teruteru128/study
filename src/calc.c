
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#define BUFSIZE (67108864)

//ファイルから先頭2行読んで16進数としてパースして掛け算する
//掛け算した結果のビット数を表示する
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "usage:%s [path]\n", argv[0]);
    return EXIT_FAILURE;
  }
  char *fname = argv[1];

  size_t i;
  mpz_t num[2];

  mpz_inits(num[0], num[1], NULL);

  {
    FILE *fp = fopen(fname, "r");
    if (fp == NULL)
    {
      perror("fopen");
      return 2;
    }
    char *buf = malloc(BUFSIZE);
    if (!buf)
    {
      perror("malloc buf");
      return EXIT_FAILURE;
    }

    memset(buf, 0, BUFSIZE);
    char *f = NULL;
    for (i = 0; i < 2 && (f = fgets(buf, BUFSIZE, fp)) != NULL;)
    {
      if (ferror(fp))
      {
        perror("fgets");
        fclose(fp);
        return 3;
      }
      char firstchar = f[0];
      if (firstchar != '#' && firstchar != '\r' && firstchar != '\n' && firstchar != '\0')
      {
        mpz_set_str(num[i], f, 10);
        i++;
      }
    }
    fclose(fp);
    free(buf);
    buf = NULL;
  }
  mpz_t p;
  mpz_t q;
  mpz_t n;
  mpz_t e;
  mpz_t d;
  mpz_t psub1;
  mpz_t qsub1;
  mpz_t phin;
  mpz_t exponent1;
  mpz_t exponent2;
  mpz_t coefficient;
  mpz_t tmp;

  // 変数初期化
  mpz_inits(p, q, n, e, d, psub1, qsub1, phin, exponent1, exponent2, coefficient, tmp, NULL);
  // pへ素数をセット
  mpz_set(p, num[0]);
  // 素数をセット
  mpz_set(q, num[1]);
  // 使い終わったtmpをクリア
  mpz_clear(tmp);
  // nを算出
  mpz_mul(n, p, q);
  fprintf(stderr, "n is %ld bits\n", mpz_sizeinbase(n, 2));
  // 公開鍵 e
  mpz_set_ui(e, 65537UL);
  // required p > q
  if (mpz_cmp(p, q) <= 0)
  {
    mpz_swap(p, q);
    fputs("p and q is swaped.\n", stdout);
  }
  // p - 1とq - 1を計算
  mpz_sub_ui(psub1, p, 1);
  mpz_sub_ui(qsub1, q, 1);
  // phi(n) を計算
  mpz_mul(phin, psub1, qsub1);
  // 秘密鍵dを計算
  mpz_invert(d, e, phin);
  // phi(n)をクリア
  mpz_clear(phin);
  fputs("d\n", stdout);
  mpz_mod(exponent1, d, psub1);
  mpz_mod(exponent2, d, qsub1);
  mpz_clears(psub1, qsub1, NULL);
  mpz_invert(coefficient, q, p);
  fputs("cof\n", stdout);

  fputs("\n", stdout);
  mpz_out_str(stdout, 16, n);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, e);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, d);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, p);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, q);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, exponent1);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, exponent2);
  fputs("\n", stdout);
  mpz_out_str(stdout, 16, coefficient);
  fputs("\n", stdout);

  mpz_clears(n, e, d, p, q, exponent1, exponent2, coefficient, NULL);
  return EXIT_SUCCESS;
}
