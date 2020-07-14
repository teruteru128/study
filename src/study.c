
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <orz.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <java_random.h>
#include <err.h>
#include <byteswap.h>

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
 */
int main(int argc, char *argv[])
{
  int64_t seed = 0x800000000000L;
  int64_t tmp;
  for (; seed < 0x800000010000L; seed++)
  {
    tmp = nInverse(seed);
    if((tmp & 0x0000ffff0000L) == 0x000000000000L) {
      printf("%ld, %ld, %012lx\n", seed, tmp, tmp);
      tmp = nInverse(tmp);
      printf("%ld\n",tmp);
      tmp = initialScramble(tmp);
      printf("%ld\n",tmp);
    }
  }
  return EXIT_SUCCESS;
}
