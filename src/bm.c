
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include "bitmessage.h"
#include "bm.h"
#include "bmapi.h"
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442
#define NAME "TR BM TEST CLIENT"
#define SERVER_URL "http://127.0.0.1:8442/"

size_t ripe(RIPE_CTX *ctx, unsigned char *signpubkey, unsigned char *encpubkey)
{
  unsigned char *cache64 = ctx->cache64;
  SHA512_Init(&ctx->sha512ctx);
  SHA512_Update(&ctx->sha512ctx, signpubkey, PUBLIC_KEY_LENGTH);
  SHA512_Update(&ctx->sha512ctx, encpubkey, PUBLIC_KEY_LENGTH);
  SHA512_Final(cache64, &ctx->sha512ctx);
  RIPEMD160_Init(&ctx->ripemd160ctx);
  RIPEMD160_Update(&ctx->ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
  RIPEMD160_Final(cache64, &ctx->ripemd160ctx);
  size_t nlz = 0;
  for (; cache64[nlz] == 0 && nlz < RIPEMD160_DIGEST_LENGTH; nlz++){}
  return nlz;
}
