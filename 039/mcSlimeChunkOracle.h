
#ifndef MC_SLIME_CHUNK_ORACLE_H
#define MC_SLIME_CHUNK_ORACLE_H

#include "random.h"
#include "stdbool.h"
typedef struct SlimeChunkSeed_t{
  uint64_t seed;
  Random rnd;
} SlimeChunkSeed;

uint64_t getMCSeed(SlimeChunkSeed*, int32_t , int32_t);
void setMCSeed(SlimeChunkSeed*, uint64_t);

bool isSlimeChunkXZ(SlimeChunkSeed *, int64_t, int64_t);
bool isSlimeChunk(Random *);

#endif

