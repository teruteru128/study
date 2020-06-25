
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>

int main(int argc, char* argv[])
{
  MD5_CTX c;
  MD5_Init (&c);

  return EXIT_SUCCESS;
}
