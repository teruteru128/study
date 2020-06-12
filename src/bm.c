
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>
#include <string.h>
#include <uuid/uuid.h>
#include "bitmessage.h"
#include "base64.h"
#include "bm.h"
#include "bmapi.h"
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442
#define NAME "TR BM TEST CLIENT"
#define SERVER_URL "http://127.0.0.1:8442/"

void die_if_fault_occurred(xmlrpc_env *env)
{
  /* Check our error-handling environment for an XML-RPC fault. */
  if (env->fault_occurred)
  {
    fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
            env->fault_string, env->fault_code);
    exit(1);
  }
}

size_t ripe(RIPE_CTX *ctx, unsigned char *signpubkey, unsigned char *encpubkey)
{
    unsigned char cache64[SHA512_DIGEST_LENGTH];
    SHA512_Init(&ctx->sha512ctx);
    SHA512_Update(&ctx->sha512ctx, signpubkey, PUBLIC_KEY_LENGTH);
    SHA512_Update(&ctx->sha512ctx, encpubkey, PUBLIC_KEY_LENGTH);
    SHA512_Final(cache64, &ctx->sha512ctx);
    RIPEMD160_Init(&ctx->ripemd160ctx);
    RIPEMD160_Update(&ctx->ripemd160ctx, cache64, SHA512_DIGEST_LENGTH);
    RIPEMD160_Final(cache64, &ctx->ripemd160ctx);
    size_t nlz = 0;
    for(;nlz < RIPEMD160_DIGEST_LENGTH; nlz++){}
    return nlz;
}
