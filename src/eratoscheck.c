
#include <gmp.h>
#include <inttypes.h>
#include <postgresql/libpq-fe.h>
#include <regex.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

/**
 * 既知素数篩を超える範囲の素数を200万ビットぐらいにまとめてGCDでぶつける
 * */
int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "%s [dbファイル] even-numberファイル\n", argv[0]);
    return 1;
  }

  char *dbFilePath = argv[1];
  char *evenNumberFilePath = argv[2];
  char pattern[] = "([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4})-[0-9a-fA-F]{"
                   "4}-[0-9a-fA-F]{12}";

  regex_t reg;
  regcomp(&reg, pattern, REG_EXTENDED);

  regmatch_t regmatch[2];
  int r = regexec(&reg, evenNumberFilePath, 2, regmatch, 0);
  if (r != 0) {
    return 1;
  }
  int start = regmatch[1].rm_so;
  int end = regmatch[1].rm_eo;
  int len = end - start;
  uint64_t msb_out;
  int success = 0;
  // 一時バッファに「8桁-4桁-4桁」の部分文字列を切り出す（サイズは18バイトあれば十分）
  char msb_part_str[32];
  snprintf(msb_part_str, sizeof(msb_part_str), "%.*s", len,
           evenNumberFilePath + start);

  // 切り出した文字列（例: "123e4567-e89b-12d3"）を16進数として解析
  uint32_t part1, part2, part3;
  if (sscanf(msb_part_str, "%8x-%4x-%4x", &part1, &part2, &part3) == 3) {
    // ビットシフトで64ビット整数に結合
    msb_out = ((uint64_t)part1 << 32) | ((uint64_t)part2 << 16) | part3;
    success = 1;
  }

  printf("%016" PRIx64 "\n", msb_out);

  char *password = getenv("PS_PASSWORD");
  if (password == NULL) {
    fprintf(stderr, "PS_PASSWORD not found\n");
    return 1;
  }
  char coninfo[1024];
  memset(coninfo, 0, 1024);
  snprintf(coninfo, 1023,
           "host=localhost port=5432 dbname=primesearch user=primesearch "
           "password=%s",
           password);

  // 5318918530104379150
  mpz_t p, ps, even, n, gcd;
  mpz_inits(p, ps, even, n, gcd, NULL);
  mpz_set_ui(p, 274877906560ULL);
  mpz_set_ui(ps, 1);
  FILE *in = fopen(evenNumberFilePath, "r");
  if (in == NULL) {
    perror("fopen");
    return 1;
  }
  mpz_inp_str(even, in, 16);
  fclose(in);
  while (1) {
    mpz_nextprime(p, p);
    uint64_t prime = mpz_get_ui(p);
    uint64_t fdiv = (uint64_t)mpz_fdiv_ui(even, prime);
    uint64_t aaaaa = prime - fdiv;
    if ((aaaaa & 1) == 0) {
      aaaaa += prime;
    }
    if (aaaaa < 13421725) {
      printf("match!: %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "\n", prime, fdiv,
             aaaaa, (aaaaa - 1) / 2);
    }
  }
  mpz_clears(p, ps, even, n, gcd, NULL);
  return 0;
}
