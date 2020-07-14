
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
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

#define BIG_PRECURE_IS_WATCHING_YOU "BIG PRECURE IS WATCHING YOU"

char *base58encode(unsigned char *input, size_t length);

struct chararray
{
  char *data;
  size_t length;
};

struct chararray *encodeVarint(uint64_t u);
void chararrayfree(struct chararray *p);
char *encodeAddress(int version, int stream, unsigned char *ripe, size_t ripelen);
size_t parseHex(unsigned char **out, const char *str);

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
  for (unsigned int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, md, len);
  EVP_DigestFinal(ctx, md, &len);
  for (unsigned int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_MD_CTX_free(ctx);
  int64_t ppp = 0x0;
  ppp = nInverse(ppp);
  printf("%ld\n", initialScramble(ppp));
  ppp = 246345201500483L;
  ppp = initialScramble(ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  // 00000D9663F57318B4E52288BFDC8B3C23E84DE1
  char *hex = "000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5";
  unsigned char *in = NULL;
  len = (unsigned int)parseHex(&in, hex);
  printf("%u\n", len);
  //memset(in, 0, 10);
  char *out = base58encode(in, len);
  printf("%s\n", out);
  free(in);
  free(out);
  char *hex2 = "00000D9663F57318B4E52288BFDC8B3C23E84DE1";
  in = NULL;
  len = (unsigned int)parseHex(&in, hex2);
  printf("%u\n", len);
  //memset(in, 0, 10);
  out = encodeAddress(4, 1, in, len);
  printf("%s\n", out);
  free(in);
  free(out);
  return EXIT_SUCCESS;
}
