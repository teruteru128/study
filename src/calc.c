
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#define BUFSIZE (67108864)

//ファイルから先頭2行読んで16進数としてパースして掛け算する
//掛け算した結果のビット数を表示する
int main(int argc, char* argv[])
{
  if(argc < 2){
    fprintf(stderr, "usage:%s [path]\n", argv[0]);
    return 1;
  }

  char* fname = argv[1];

  size_t i;
  mpz_t num[2];

  mpz_inits(num[0], num[1], NULL);

  {
    FILE* fp = fopen(fname, "r");
    if(fp == NULL){
      perror("fopen");
      return 2;
    }
    char buf[BUFSIZE];

    memset(buf, 0, BUFSIZE);
    char *f = NULL;
    for(i = 0; i < 2 && (f = fgets(buf, BUFSIZE, fp)) != NULL; ) {
      if(ferror(fp)){
        perror("fgets");
        fclose(fp);
        return 3;
      }
      char firstchar = f[0];
      if(firstchar != '#' && firstchar != '\r' && firstchar != '\n' && firstchar != '\0') {
        mpz_set_str(num[i], f, 16);
        i++;
      }
    }
    fclose(fp);
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
  // 素数探索
  mpz_nextprime(tmp, num[0]);
  fprintf(stderr, "p is prime\n");
  // pへ素数をセット
  mpz_set(tmp, p);
  // 素数探索
  mpz_nextprime(tmp, num[1]);
  fprintf(stderr, "q is prime\n");
  // 素数をセット
  mpz_set(tmp, q);
  // 使い終わったtmpをクリア
  mpz_clear(tmp);
  // nを算出
  mpz_mul(n, p, q);
  fprintf(stderr, "n is %ld bits\n", mpz_sizeinbase(n, 2));
  // 公開鍵 e
  mpz_set_ui(e, 65537UL);
  // required p > q
  if(mpz_cmp(p, q) <= 0){
    mpz_swap(p, q);
    fputs("p and q is swaped.", stderr);
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
  fputs("d", stderr);
  mpz_mod(exponent1, d, psub1);
  mpz_mod(exponent2, d, qsub1);
  mpz_clears(psub1, qsub1, NULL);
  mpz_invert(coefficient, q, p);
  fputs("cof", stderr);

  mpz_out_str(stdout, 16, n);
  puts("");
  mpz_out_str(stdout, 16, e);
  puts("");
  mpz_out_str(stdout, 16, d);
  puts("");
  mpz_out_str(stdout, 16, p);
  puts("");
  mpz_out_str(stdout, 16, q);
  puts("");
  mpz_out_str(stdout, 16, exponent1);
  puts("");
  mpz_out_str(stdout, 16, exponent2);
  puts("");
  mpz_out_str(stdout, 16, coefficient);
  puts("");

  mpz_clears(n, e, d, p, q, exponent1, exponent2, coefficient, NULL);

  return 0;
}

