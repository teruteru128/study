
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <gmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief サイコロによる6進数をバイナリに変換するツール
 * dicetobin(unsigned char *bin, size_t s, char *dice)
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) {
  mpz_t hex;
  mpz_init(hex);

  size_t size = 256;
  char *dice = malloc(size);
  memset(dice, 0, size);
  size_t length = 0;

  int c;
  int con = 1;
  printf("'c'でリセットします。\n");
  printf("'q', 'f' のいずれかで終了します。\n");
  while (1) {
    c = fgetc(stdin);

    // 1. 終了・エラー判定
    if (c == EOF || strchr("fq", c)) {
      if (c == EOF && ferror(stdin)) {
        perror("fgetc");
      }
      break;
    }

    // 1.1. リセット処理
    if (c == 'c') {
      length = 0;
      if (dice)
        dice[0] = '\0';
      mpz_set_ui(hex, 0);
      printf("--- Data Reset ---\n");
    }

    // 2. 有効な入力（1-6）
    if (c >= '1' && c <= '6') {
      if (length + 1 == size) {
        size *= 2;
        char *tmp = realloc(dice, size);
        if (!tmp) {
          perror("realloc");
          free(dice);
          exit(EXIT_FAILURE);
        }
        dice = tmp;
      }

      dice[length++] = (char)c;
      dice[length] = '\0'; // 終端保証

      mpz_mul_ui(hex, hex, 6);
      mpz_add_ui(hex, hex, (unsigned long)(c - '1'));
      continue;
    }

    // 4. 無視する文字（空白など）とエラー
    if (isspace(c)) {
      if (c == '\n' && length > 0) {
        gmp_printf("(%zudigits)%s\n(%zubit)%Zx\n", length, dice,
                   mpz_sizeinbase(hex, 2), hex);
      }
      continue;
    }
    fprintf(stderr, "invarit charactor!: (0x%02x)\n", c);
  }

  // 終了直前
  if (dice) {
    memset(dice, 0, size); // メモリ上の機密情報を消去
    free(dice);
    dice = NULL;
  }

  mpz_clear(hex);
  return 0;
}
