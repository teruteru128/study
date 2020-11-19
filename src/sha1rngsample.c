
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <openssl/evp.h>

#define LIMIT 16

typedef struct rndctx
{
  uint64_t count;
  unsigned char ctx[20];
} rndctx_t;

int64_t next(rndctx_t *ctx)
{
  const EVP_MD *sha1 = EVP_sha1();
  // 毎回ctxをnewしてfreeするのもったいないけどなにかいい方法ない？
  EVP_MD_CTX *tmpmd = EVP_MD_CTX_new();
  EVP_DigestInit(tmpmd, sha1);
  *((uint64_t *)(ctx->ctx + 12)) = htobe64(ctx->count);
  ctx->count++;
  EVP_DigestUpdate(tmpmd, ctx->ctx, 20);
  EVP_DigestFinal(tmpmd, ctx->ctx, NULL);
  EVP_MD_CTX_free(tmpmd);
  return be64toh(*((uint64_t *)(ctx->ctx + 12)));
}

/**
 * sha1 rng sample
 */
int main(int argc, char *argv[])
{
  /*
    SHA-1ベース乱数
    内部状態 12バイト乱数+8バイトカウンタ(0スタート)
    初期化 12バイト乱数
    sha1(12バイト乱数+8バイトカウンタ)
    →12バイト再利用+8バイト出力
    カウンタをctxに上書き
    カウンタをインクリメント
    ハッシュしてctxに書き込む
    出力値を返す
  */
  rndctx_t ctx;
  ctx.count = 0;
  unsigned char buf[20];
  FILE *r = fopen("/dev/urandom", "rb");
  if (r == NULL)
  {
    return EXIT_FAILURE;
  }
  size_t req = 12;
  size_t len = fread(buf, sizeof(char), req, r);
  if (len != req)
  {
    perror("fread error1");
  }
  len = fread(ctx.ctx, sizeof(char), req, r);
  if (len != req)
  {
    perror("fread error2");
  }
  fclose(r);
  *((uint64_t *)(buf + 12)) = htobe64(1);
  for (size_t i = 0; i < 20; i++)
  {
    printf("%02x", buf[i]);
  }
  fputs("\n", stdout);
  for (int i = 0; i < 20; i++)
  {
    printf("%ld\n", next(&ctx));
  }
  return EXIT_SUCCESS;
}
