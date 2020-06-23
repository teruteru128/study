
#ifndef BITSET_H
#define BITSET_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// http://hg.openjdk.java.net/jdk/jdk/file/fb39a8d1d101/src/java.base/share/classes/java/util/BitSet.java
typedef struct bitset_t bitset;
typedef struct bitset_t
{
  size_t wordsInUse;
  bool sizeIsSticky;
  size_t words_length;
  uint64_t *words;
} bitset;
void checkInvariants(bitset *set);
//static size_t wordIndex(size_t bitIndex);
bool bitset_get(bitset *set, size_t bitIndex);
void bitset_init(bitset *set);
void bitset_init2(bitset *set, size_t nbits);
/*bitset *bitset_alloc()*/
void bitset_free(bitset *set);
void bitset_set(bitset *set, size_t bitIndex);
#endif
