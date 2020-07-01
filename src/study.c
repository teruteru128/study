
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
  /*
  int64_t x2 = x * x * 0x4c1906;
  int64_t x1 = x * 0x5ac0db;
  int64_t z2 = z * z * 0x4307a7L;
  int64_t z1 = z * 0x5f24f;
  return (seed + x2 + x1 + z2 + z1) ^ 0x3ad8025f;
  */
}

static int64_t t(int64_t seed, int32_t x, int32_t z)
{
  return (seed + (long)(x * x * 0x4c1906) + (long)(x * 0x5ac0db) + (long)(z * z) * 0x4307a7L + (long)(z * 0x5f24f));
}

int isSlimeChunk(int64_t seed, int32_t x, int32_t z)
{
  int64_t seed1 = s(seed, x, z);
  return !nextIntWithBounds(&seed1, 10);
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
  printf("%016lx\n", seed);
  seed = initialScramble(seed);
  printf("%ld\n", n(seed) >> 31);
  int64_t rnd = 0;
  //setSeed(&rnd, s(42056176818708L, -196, -150));
  rnd = initialScramble(s(42056176818708L, -196, -150));
  printf("%d\n", nextIntWithBounds(&rnd, 10));
  setSeed(&rnd, s(42056176818708L, -195, -149));
  printf("%d\n", nextIntWithBounds(&rnd, 10));
  setSeed(&rnd, s(42056176818708L, -194, -148));
  printf("%d\n", nextIntWithBounds(&rnd, 10));
  setSeed(&rnd, s(42056176818708L, -193, -147));
  printf("%d\n", nextIntWithBounds(&rnd, 10));
  seed = initialScramble(s(42056176818708L, -196, -150));
  printf("%d\n", nextIntWithBounds(&seed, 10));
  seed = 42056176818708L;
  printf("%016lx\n", t(seed, -196, -150));
  printf("%016lx\n", t(seed, -195, -149));
  printf("%016lx\n", t(seed, -194, -158));
  printf("%016lx\n", t(seed, -193, -147));
  return EXIT_SUCCESS;
}
