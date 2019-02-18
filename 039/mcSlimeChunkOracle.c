
#include <stdio.h>
#include "mcSlimeChunkOracle.h"

void setMCSeed(SlimeChunkSeed* t, uint64_t seed){
    t->seed = seed;
}

uint64_t getMCSeed(SlimeChunkSeed* seed, int32_t chunkX, int32_t chunkZ){
  return seed->seed + chunkX * chunkX * 0x4c1906 + chunkX * 0x5ac0db + chunkZ * chunkZ * 0x4307a7L + chunkZ * 0x5f24f ^ 0x3ad8025f;
}

bool isSlimeChunk(Random* rnd){
    return nextIntWithRange(rnd, 10) == 0;
}

bool isSlimeChunkXZ(SlimeChunkSeed* seed, int64_t chunkX, int64_t chunkZ){
  Random* rnd = &seed->rnd;
  setSeed(rnd, getMCSeed(seed, (int32_t)chunkX, (int32_t)chunkZ));
  return isSlimeChunk(rnd);
}

