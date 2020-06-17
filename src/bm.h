
#ifndef BM_H
#define BM_H

#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>

#define PUBLIC_KEY_LENGTH 65

typedef struct
{
  SHA512_CTX sha512ctx;
  RIPEMD160_CTX ripemd160ctx;
  unsigned char cache64[SHA512_DIGEST_LENGTH];
} RIPE_CTX;

typedef struct clientinfo_t
{
} clientinfo;

typedef struct serverinfo_t
{
} serverinfo;

typedef struct connectioninfo_t
{
  clientinfo client;
  serverinfo server;
} connectioninfo;

/* int ripe(ripectx, signpub, encpub) */
size_t ripe(RIPE_CTX *, unsigned char *signpub, unsigned char *encpub);

#endif
