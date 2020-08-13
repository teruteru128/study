
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

static size_t calcArrayOffset(int32_t x, int32_t z)
{
  return (z + 625) * 1250 + x + 625;
}

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
  clock_gettime(CLOCK_REALTIME, &start);
  int64_t ctx;
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
  long nsec = end.tv_nsec - start.tv_nsec;
  long sec = end.tv_sec - start.tv_sec;
  if (nsec < 0)
  {
    sec -= 1;
    nsec += 1000000000;
  }
  printf("%f\n", sec + (nsec / 1e9));
  free(set);
}
