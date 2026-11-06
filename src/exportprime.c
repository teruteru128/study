
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>

// 16進数文字列からstep*2+1を足して素数にして書き出すわよ
int main(int argc, char *argv[]) {
  if(argc < 3){
    fprintf(stderr, "おお\n");
    return 1;
  }
  FILE* fin = fopen(argv[1], "r");
  if(fin == NULL){
    return 1;
  }
  mpz_t pp, a;
  mpz_inits(pp, a, NULL);
  mpz_inp_str(pp, fin, 16);
  fclose(fin);
  mpz_add_ui(pp, pp, strtol(argv[2], NULL, 10) * 2 + 1);
  fprintf(stderr, "%zu bits\n", mpz_sizeinbase(pp, 2));
  gmp_printf("%Zu\n", pp);
  mpz_clears(pp, a, NULL);

  return 0;
}
