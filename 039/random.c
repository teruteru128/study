
#include "internal_random.h"
#include "random.h"

static uint64_t initialScramble(uint64_t seed){
    return (seed ^ MULTIPLIER) & MASK;
}

Random* setSeed(Random *rnd, uint64_t seed){
    rnd->seed = initialScramble(seed);
    return rnd;
}

uint32_t next(Random* rnd, uint32_t bits){
    uint64_t oldseed, nextseed;
    uint64_t seed = rnd->seed;
    oldseed = seed;
    nextseed = (oldseed * MULTIPLIER + ADDEND) & MASK;
    rnd->seed = nextseed;
    return (uint32_t)(nextseed >> (48 - bits));
}

uint64_t nextLong(Random* rnd){
    return ((uint64_t)(next(rnd, 32)) << 32) + next(rnd, 32);
}

uint32_t nextInt(Random* rnd){
    return next(rnd, 32);
}
uint32_t nextIntWithRange(Random *rnd, uint32_t bound){
    if(bound <= 0){
      // err!
      return 0;
    }
    uint32_t r = next(rnd, 31);
    uint32_t m = bound - 1;
    if((bound & m) == 0){
        r = (uint32_t)((bound * (uint64_t)r) >> 31);
    }else{
      uint32_t u;
      for(u = r; u - (r = u % bound) + m < 0; u = next(rnd, 31));
    }
    return r;
}

