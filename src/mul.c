
#include <stdio.h>
#include <gmp.h>

/* 
 * even number同士をかけあわせてビット数を確認する
 * 最初のステップでかけあわせた結果のビット長を確認していなかったため。
 * */
int main(int argc, char *argv[]) {
  if(argc < 3) {
    return 1;
  }
  mpz_t n, p, q;
  mpz_inits(n, p, q, NULL);
  FILE *file1, *file2;
  file1 = fopen(argv[1], "r");
  file2 = fopen(argv[2], "r");
  if(file1 == NULL || file2 == NULL) {
    fclose(file1);
    fclose(file2);
    return 1;
  }
  mpz_inp_str(p, file1, 16);
  mpz_inp_str(q, file2, 16);
  fclose(file1);
  fclose(file2);
  mpz_mul(n, p, q);
  printf("%zu bits\n", mpz_sizeinbase(n, 2));
  mpz_clears(n, p, q, NULL);
  return 0;
}
