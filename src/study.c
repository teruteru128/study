
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <openssl/bn.h>
#include <openssl/err.h>

#define STR "0111011101011100010010111001001000110110001111111110101110001011000010101100010110101111001010111000111101111111100101011111000011111110010100100101111000000000010000010010000111110110001001011000100101101111111010111001100010101100111001101100001111111011100100010111110000111111011100011010110011100001110111001110101111110101100010010001001011110100001000101001000010000010111001110011100110110010001000111101010110010010010111000001000010101011100101000010001010010110010100011010110001010000100001100011000100011100101110010000100010010000111000111010111011010010001000001100001001000011111010110010011011010111000011010010010000100100111100000101001100001000111110111001111000100101011111100101011000000011011000101011000111100101101101011011110111110010100101101001110011111101000010000101011110010100011011011100010101010100010010010000000010100110100001101011101011010010011110011110001001110001101101010010100101101010100000101101010110010100111111111100100011111010100010110110111100001001011100110001000100011111"
#define BASE16 "0123456789abcdef"

static int callback(int a, int b, BN_GENCB *cb)
{
  void *ptr = BN_GENCB_get_arg(cb);
  printf("(´・ω・｀)おほーっ %d, %d, %p, %p\n", a, b, cb, ptr);
  return 1;
}

/**
 * --version
 * --help
 * 
 * orz
 * 
 * OpenSSL EVP
 * 対称鍵暗号
 * 認証付き暗号
 * エンベロープ暗号化
 * 署名と検証
 *   EVP_DigestSign
 * メッセージダイジェスト
 * 鍵合意(鍵交換)
 * メッセージ認証符号 (OpenSSL 3～)
 *   EVP_MAC_new_ctx
 * 鍵導出関数
 * strpbrk
 *   文字検索関数
 * strsep
 *   トークン分割(空フィールド対応版)
 */
int main(int argc, char *argv[])
{
  size_t i = 0;
  size_t j = 0;
  int k = 0;
  char l[257];
  memset(l, 0, 257);
  size_t m;
  for (i = 0; i < 256; i++)
  {
    m = i << 2;
    for (j = 0, k = 0; j < 4; j++)
    {
      k = (k << 1) + (STR[m + j] & 1);
    }
    l[i] = BASE16[k];
  }
  printf("%s\n", l);

  BN_CTX *ctx = BN_CTX_new();
  BN_CTX_start(ctx);
  BIGNUM *p = BN_CTX_get(ctx);
  BN_hex2bn(&p, l);
  BN_GENCB *gencb = BN_GENCB_new();
  BN_GENCB_set(gencb, callback, p);
  int r = BN_is_prime_fasttest_ex(p, BN_prime_checks, ctx, 1, gencb);
  printf("%d\n", r);
  if (r == -1)
  {
    unsigned long err = ERR_get_error();
    fprintf(stderr, "BN_is_prime_fasttest_ex : %s\n", ERR_error_string(err, NULL));
    return EXIT_FAILURE;
  }
  BN_GENCB_free(gencb);
  BN_CTX_end(ctx);
  BN_CTX_free(ctx);
  return EXIT_SUCCESS;
}
