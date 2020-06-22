
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include "gettextsample.h"
#include "printint.h"
#include "random.h"
#include "bitset.h"
#include "orz.h"

// 
// nextもrandomも使われているので適当にpとする
int64_t p(int64_t seed) {
  return (seed * 0x5DEECE66DL + 0xBL) & 0xFFFFFFFFFFFFL;
}

int64_t pInverse(int64_t seed) {
  return (seed - 0xBL) * 0xDFE05BCB1365L & 0xFFFFFFFFFFFFL;
}

int64_t initializeSeed(int64_t seed) {
  return (seed ^ 0x5DEECE66DL) & 0xFFFFFFFFFFFFL;
}

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
  orz(1);

  char *data = malloc(100);
  printf("%lu\n", malloc_usable_size(data));
  free(data);
  int64_t seed = 0x0;
  seed = pInverse(seed);
  printf("%ld\n", initializeSeed(seed));
  seed = pInverse(seed);
  printf("%ld\n", initializeSeed(seed));
  seed = pInverse(seed);
  printf("%ld\n", initializeSeed(seed));
  return EXIT_SUCCESS;
}
