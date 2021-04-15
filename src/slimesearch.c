
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <regex.h>
#include <stdint.h>
#include <minecraft.h>
#include <time.h>
#include "timeutil.h"
#include <math.h>

#define unitIndex(bitIndex) ((bitIndex) >> 6)
#define bit(bitIndex) (1UL << ((bitIndex) & ((1 << 6) - 1)))

/* http://tdual.hatenablog.com/entry/2018/05/02/113110 */
struct searchArea
{
  int64_t seed_start;
  int64_t seed_end;
  int32_t input_x;
  int32_t input_z;
  int32_t kernel_x;
  int32_t kernel_z;
  int32_t threshold;
};

#define calcArrayOffset(x, z) (((z) + 625) * 1250 + (x) + 625)

void *searchTask(void *arg)
{
  // 0~624
  // -625 ~ -1
  size_t index = calcArrayOffset(0, 0);
  /*
   * 1250z + 1250 * 625 + x + 625
   * = x + 625 (2z + 1251)
   */
  size_t word = index >> 6;
  size_t bitindex = index & 0x3f;
  uint64_t set[24415];
  int64_t ctx;
  set[word] |= (isSlimeChunk(&ctx, 0, 0, 0) & 0x01) << bitindex;
  return NULL;
}

void startMCSlimeChunkSearch()
{
  uint64_t *set = calloc(24415, sizeof(uint64_t));
  if (!set)
  {
    perror("calloc");
    return 1;
  }

  // 0~624
  // -625 ~ -1
  size_t index = 0;
  /*
   * 1250z + 1250 * 625 + x + 625
   * = x + 625 (2z + 1251)
   */
  size_t word = 0;
  size_t bitindex = 0;
  struct timespec start;
  struct timespec end;
  struct timespec diff;
  int64_t ctx;
  clock_gettime(CLOCK_REALTIME, &start);
  for (int32_t z = -625; z < 625; z++)
  {
    for (int32_t x = -625; x < 625; x++)
    {
      index = calcArrayOffset(x, z);
      word = index >> 6;
      bitindex = index & 0x3f;
      //printf("index: %ld, word: %ld, bitindex: %ld\n", index, word, bitindex);
      set[word] |= (isSlimeChunk(&ctx, 0, x, z) & 0x01) << bitindex;
    }
  }
  clock_gettime(CLOCK_REALTIME, &end);
  difftimespec(&diff, &end, &start);
  printf("%f\n", fma(diff.tv_sec, 1e9, diff.tv_nsec) / 1e9);
  free(set);
}

int main(void)
{
  return 0;
}
