
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
  PGconn *conn = PQconnectdb(coninfo);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "接続エラー: %s", PQerrorMessage(conn));
    PQfinish(conn);
    return 1;
  }
  char id[30];
  snprintf(id, 29, "%" PRIu64, msb_out);
  int nParams = 1;
  const char *paramValues[1];
  paramValues[0] = id;
  char *query = "SELECT step from candidates where id = $1 and composite = 0 "
                "and timeassigned is null;";
  PGresult *res = PQexecParams(
      conn, query,
      nParams,     // パラメータ数
      NULL,        // パラメータの型 OID 配列 (NULL で自動推論)
      paramValues, // パラメータの値
      NULL,        // パラメータの長さ (テキストモードならNULLで可)
      NULL, // パラメータのフォーマット (0=テキスト, 1=バイナリ / NULLで0)
      0     // 結果のフォーマット (0=テキスト, 1=バイナリ)
  );
  // 5. エラーチェックと結果の取得
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "クエリ実行失敗: %s", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    exit(1);
  }
  int nrows = PQntuples(res);

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
  for (int i = 0; i < 55190; i++) {
    mpz_nextprime(p, p);
    mpz_mul(ps, ps, p);
  }
  for (int i = 0; i < nrows; i++) {
    mpz_add_ui(n, even, strtoull(PQgetvalue(res, i, 0), NULL, 10) * 2 + 1);
    mpz_gcd(gcd, n, ps);
    if (mpz_cmp_ui(gcd, 1) != 0) {
      gmp_printf("%s is composite\n", PQgetvalue(res, 1, 0));
    }
  }
  mpz_clears(p, ps, even, n, gcd, NULL);
  PQclear(res);
  PQfinish(conn);
  return 0;
}
