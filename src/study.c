
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include "gettextsample.h"
#include <printint.h>
#include <random.h>
#include <java_random.h>
#include <bitset.h>
#include <orz.h>
#include <openssl/evp.h>

static int64_t s(int64_t seed, int32_t x, int32_t z)
{
  return (seed + (long)(x * x * 0x4c1906) + (long)(x * 0x5ac0db) + (long)(z * z) * 0x4307a7L + (long)(z * 0x5f24f)) ^ 0x3ad8025f;
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
 */
int main(int argc, char *argv[])
{
  int64_t seed = 42056176818708L;
  seed = s(seed, -196, -150);
  printf("%ld\n", seed);
  seed = initializeSeed(seed);
  printf("%ld\n", s(0, 0, 0));
  printf("%ld\n", s(0, 0, 1));
  printf("%ld\n", s(0, 0, 2));
  printf("%ld\n", s(0, 0, 3));
  return EXIT_SUCCESS;
}
