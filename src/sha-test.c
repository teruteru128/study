
#include <string.h>
#include "sha.h"

int main(int argc, char* argv[]){

  SHA_CTX sha;

  sha_init(&sha);
  char data[] = "The quick brown fox jumps over the lazy dog";
  sha_update(&sha, data, strlen(data));
  
  char buf[20];
  char txt[41];
  sha_finish(buf, &sha);

  return EXIT_SUCCESS;
}