
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

/**
 * --version
 * --help
 * 
 * orz
 */
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Help me!\n"));
  const EVP_MD *sha512 = EVP_sha512();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, "BIG PRECURE IS WATCHING YOU", strlen("BIG PRECURE IS WATCHING YOU"));
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int len = 0;
  EVP_DigestFinal(ctx, md, &len);
  for(int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, md, len);
  EVP_DigestFinal(ctx, md, &len);
  for(int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  return EXIT_SUCCESS;
}
