
#ifndef BM_H
#define BM_H

#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>

typedef struct
{
    SHA512_CTX sha512ctx;
    RIPEMD160_CTX ripemd160ctx;
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

typedef struct bm_client_t
{
  xmlrpc_env *env;
  xmlrpc_client *cp;
  xmlrpc_server_info *sinfo;
} bm_client_t;

/* int ripe(ripectx, signpub, encpub) */
int ripe(RIPE_CTX *, char *signpub, size_t offsetsingpub, size_t lengthsignpub, char *encpub, size_t offsetencpub, size_t lengthencpub);

#endif
