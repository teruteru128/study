
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/md5.h>

int main(int argc, char **argv)
{
  const EVP_MD *md5 = EVP_md5();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  char str[22];
  unsigned char buf[16];
  unsigned char target[16];
  size_t len = 0;
  memset(target, 0, 16);
  size_t j = 4;
  for (size_t i = 776869784885229UL; i < 776869784885229UL + INT_MAX; i++)
  {
    len = (size_t)snprintf(str, 22, "%zu", i);
    EVP_DigestInit(ctx, md5);
    EVP_DigestUpdate(ctx, str, len);
    EVP_DigestFinal(ctx, buf, NULL);
#if 0
    if (buf[0])
      continue;
    if (buf[1])
      continue;
    if (buf[2])
      continue;
    if (buf[3])
      continue;
#endif
    if (memcmp(buf, target, j))
      continue;
    fprintf(stdout, "%zu\n", i);
    j++;
    if (j >= 16)
      break;
  }
  EVP_MD_CTX_free(ctx);

  return EXIT_SUCCESS;
}
