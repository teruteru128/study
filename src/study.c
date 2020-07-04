
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include "gettextsample.h"
#include <printint.h>
#include <random.h>
#include <bitset.h>
#include <orz.h>
#include <openssl/evp.h>

#define BIG_PRECURE_IS_WATCHING_YOU "BIG PRECURE IS WATCHING YOU"

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
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Help me!\n"));
  orz(1);
  const EVP_MD *sha512 = EVP_sha512();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, BIG_PRECURE_IS_WATCHING_YOU, strlen(BIG_PRECURE_IS_WATCHING_YOU));
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int len = 0;
  EVP_DigestFinal(ctx, md, &len);
  for (int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, md, len);
  EVP_DigestFinal(ctx, md, &len);
  for (int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_MD_CTX_free(ctx);
  return EXIT_SUCCESS;
}
