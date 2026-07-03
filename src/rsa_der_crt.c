#include <gmp.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DERファイルを丸ごとメモリに読み込む関数
unsigned char *load_der_file(const char *filepath, size_t *out_len) {
  FILE *fp = fopen(filepath, "rb"); // バイナリモードで開く
  if (!fp) {
    perror("DERファイルを開けません");
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  unsigned char *buffer = malloc(fsize);
  if (!buffer) {
    fprintf(stderr, "メモリ確保に失敗しました。\n");
    fclose(fp);
    return NULL;
  }

  fread(buffer, 1, fsize, fp);
  fclose(fp);
  *out_len = fsize;
  return buffer;
}

// PKCS#1 DERバイナリから、順番に現れるINTEGERをGMP変数配列に格納する関数
int parse_pkcs1_private_key(const unsigned char *der, size_t der_len,
                            mpz_t *elements, int max_elements) {
  int count = 0;
  size_t i = 0;

  // シークエンス（0x30）のヘッダーをスキップ
  if (der[i] == 0x30) {
    i++;
    if (der[i] & 0x80) {
      i += (der[i] & 0x7F) + 1;
    } else {
      i++;
    }
  }

  while (i < der_len && count < max_elements) {
    if (der[i] == 0x02) { // INTEGER タグ
      i++;
      size_t len = 0;
      if (der[i] & 0x80) { // 長さが複数バイトに及ぶ場合
        int len_bytes = der[i] & 0x7F;
        i++;
        for (int j = 0; j < len_bytes; j++) {
          len = (len << 8) | der[i++];
        }
      } else {
        len = der[i++];
      }

      // GMP変数へビッグエンディアンとしてインポート
      mpz_import(elements[count], len, 1, 1, 1, 0, &der[i]);
      count++;
      i += len;
    } else {
      i++;
    }
  }
  return count;
}

int main() {
  // GMP変数の初期化
  mpz_t m, c;
  mpz_t n, e, d, p, q, dp, dq, qinv;
  mpz_t m1, m2, h;

  mpz_inits(m, c, n, e, d, p, q, dp, dq, qinv, m1, m2, h, NULL);

  // 1. DERファイルの読み込み (ファイル名は環境に合わせて変更してください)
  size_t der_len = 0;
  unsigned char *der_bytes = load_der_file("private_key.der", &der_len);
  if (!der_bytes)
    return 1;

  // パース用の一時配列
  int max_el = 9;
  mpz_t *raw_elements = malloc(max_el * sizeof(mpz_t));
  for (int i = 0; i < max_el; i++)
    mpz_init(raw_elements[i]);

  // 2. DERから9つの巨大整数を一網打尽にパース
  int found = parse_pkcs1_private_key(der_bytes, der_len, raw_elements, max_el);
  free(der_bytes);

  if (found < 9) {
    fprintf(
        stderr,
        "エラー: PKCS#1構造に必要なパラメータ(9個)が足りません。(検出: %d個)\n",
        found);
    fprintf(stderr,
            "※もしPKCS#"
            "8形式の場合は先頭のメタデータのせいでズレる可能性があります。\n");
    return 1;
  }

  // 3. 構造体の定義通りに各変数にコピー
  mpz_set(n, raw_elements[1]);    // モジュラス (2097152 bit)
  mpz_set(e, raw_elements[2]);    // 公開指数 (通常 65537)
  mpz_set(d, raw_elements[3]);    // 秘密鍵のd (2097152 bit)
  mpz_set(p, raw_elements[4]);    // 素数p (1048576 bit)
  mpz_set(q, raw_elements[5]);    // 素数q (1048576 bit)
  mpz_set(dp, raw_elements[6]);   // d mod (p-1)
  mpz_set(dq, raw_elements[7]);   // d mod (q-1)
  mpz_set(qinv, raw_elements[8]); // q^-1 mod p

  printf("--- 2097152bit DER鍵パラメータの自動抽出に成功 ---\n");
  printf("Modulus N: %lu bits\n", (unsigned long)mpz_sizeinbase(n, 2));
  printf("Prime P  : %lu bits\n", (unsigned long)mpz_sizeinbase(p, 2));
  printf("Prime Q  : %lu bits\n", (unsigned long)mpz_sizeinbase(q, 2));

  // 4. テキストを暗号化 (N未満なら200KB超の巨大なファイル等でも暗号化可能です)
  const char *secret_message = "100万ビット素数2つからなる、2097152bitの自作DER"
                               "秘密鍵によるCRT高速復号テスト成功！";
  mpz_import(m, strlen(secret_message), 1, 1, 1, 0, secret_message);

  printf("\n[1/2] 公開鍵による暗号化を実行中 (c = m^e mod n)...\n");
  mpz_powm(c, m, e, n);
  printf("→ 暗号化完了。\n");

  // 5. 中国人の剰余定理（CRT）による超高速復号
  printf("\n[2/2] 秘密鍵(CRTパラメータ)による高速復号を実行中...\n");

  // OMPのパラレルセクションで2つの冪乗を同時に走らせる
#pragma omp parallel sections
  {
#pragma omp section
    {
      mpz_powm(m1, c, dp, p); // m1 = c^dp mod p
    }
#pragma omp section
    {
      mpz_powm(m2, c, dq, q); // m2 = c^dq mod q
    }
  }
  mpz_sub(h, m1, m2);
  if (mpz_sgn(h) < 0)
    mpz_add(h, h, p);
  mpz_mul(h, h, qinv);
  mpz_mod(h, h, p); // h = (m1 - m2) * qinv mod p

  mpz_mul(m, h, q);
  mpz_add(m, m, m2); // m = m2 + h * q
  printf("→ 復号完了。\n");

  // 6. 復号結果の表示
  size_t count;
  char *decrypted_text = mpz_export(NULL, &count, 1, 1, 1, 0, m);
  printf("\n【復号されたメッセージ】:\n%.*s\n", (int)count, decrypted_text);

  // 後片付け
  free(decrypted_text);
  for (int i = 0; i < max_el; i++)
    mpz_clear(raw_elements[i]);
  free(raw_elements);
  mpz_clears(m, c, n, e, d, p, q, dp, dq, qinv, m1, m2, h, NULL);
  return 0;
}
