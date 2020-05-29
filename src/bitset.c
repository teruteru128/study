
#include "bitset.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define ADDRESS_BITS_PER_WORD (6)
#define BITS_PER_WORD (1 << ADDRESS_BITS_PER_WORD)
#define BIT_INDEX_MASK (BITS_PER_WORD - 1)

void checkInvariants(bitset *set)
{
  assert(set->wordsInUse == 0 || set->words[set->wordsInUse - 1] != 0);
  assert(set->wordsInUse >= 0 && set->wordsInUse <= set->words_length);
  assert(set->wordsInUse == set->words_length || set->words[set->wordsInUse] == 0);
}

static size_t wordIndex(size_t bitIndex)
{
  return bitIndex >> ADDRESS_BITS_PER_WORD;
}

static void ensureCapacity(bitset *set, size_t wordsRequired)
{
  if (set->words_length < wordsRequired)
  {
    size_t request = 2 * (set->words_length) > (wordsRequired) ? (set->words_length) : (wordsRequired);
    uint64_t *tmp = realloc(set->words, sizeof(uint64_t) * request);
    if (tmp != NULL)
    {
      memset(tmp + set->words_length, 0, sizeof(uint64_t) * (request - set->words_length));
      set->words_length = request;
      set->sizeIsSticky = false;
      set->words = tmp;
    }
  }
}

static void expandTo(bitset *set, size_t wordIndex)
{
  size_t wordsRequired = wordIndex + 1;
  if (set->wordsInUse < wordsRequired)
  {
    ensureCapacity(set, wordsRequired);
    set->wordsInUse = wordsRequired;
  }
}

bool bitset_get(bitset *set, size_t bitIndex)
{
  checkInvariants(set);
  size_t index = wordIndex(bitIndex);
  return (index < set->wordsInUse) && ((set->words[index] & (1L << bitIndex)) != 0);
}

static void initWords(bitset *set, size_t nbits)
{
  set->words_length = wordIndex(nbits - 1) + 1;
  set->words = malloc(sizeof(uint64_t) * set->words_length);
  memset(set->words, 0, sizeof(uint64_t) * set->words_length);
}

void bitset_init(bitset *set)
{
  set->wordsInUse = 0;
  initWords(set, BITS_PER_WORD);
  set->sizeIsSticky = false;
}

void bitset_init2(bitset *set, size_t nbits)
{
  if (nbits < 0)
  {
    return;
  }
  initWords(set, nbits);
  set->sizeIsSticky = true;
}

#if 0
bitset* bitset_alloc()
{
  bitset* set = malloc(sizeof(bitset));
  memset(set, 0, sizeof(bitset));
  bitset_init(set);
  return set;
}
#endif

void bitset_free(bitset *set)
{
  if (set->words != NULL)
  {
    free(set->words);
    set->words = NULL;
  }
}

void bitset_set(bitset *set, size_t bitIndex)
{
  size_t index = wordIndex(bitIndex);
  expandTo(set, index);
  set->words[index] |= (1L << bitIndex);

  checkInvariants(set);
}
