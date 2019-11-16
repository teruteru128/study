
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "random.h"
#include "internal_random.h"

uint32_t xor(const uint32_t seed){
	uint32_t s = seed;
	s = s ^ (s << 13);
	s = s ^ (s >> 17);
	return s ^ (s << 15);
}
uint64_t xor64(const uint64_t seed){
	uint64_t s = seed;
	s = s ^ (s << 13);
	s = s ^ (s >> 7);
	return s ^ (s << 17);
}
uint32_t xor96(const uint32_t seed1, const uint32_t seed2, const uint32_t seed3){
	return 0;
}
uint32_t xor128(const uint32_t seed1, const uint32_t seed2, const uint32_t seed3, const uint32_t seed4){
	return 0;
}

int get_random(const char* const path, void* buf, const size_t size, const size_t nmemb) {

	FILE* rnd = fopen(path, "rb");
	if(rnd == NULL){
		perror("fopen rnd");
		return -1;
	}
	size_t r = fread(buf, size, nmemb, rnd);
	if(r < 0){
		perror("fread rnd");
		return -1;
	}
	if(r !=  nmemb){
		warnx("reading failed");
	}

	(void) fclose(rnd);
	return 0;
}

static int64_t initialScramble(uint64_t seed){
    return (seed ^ MULTIPLIER) & MASK;
}

Random* setSeed(Random *rnd, int64_t seed){
    rnd->seed = initialScramble(seed);
    return rnd;
}

int32_t next(Random* rnd, int32_t bits){
    uint64_t oldseed, nextseed;
    uint64_t seed = rnd->seed;
    oldseed = seed;
    nextseed = (oldseed * MULTIPLIER + ADDEND) & MASK;
    rnd->seed = nextseed;
    return (uint32_t)(nextseed >> (48 - bits));
}

int64_t nextLong(Random* rnd){
    return ((int64_t)(next(rnd, 32)) << 32) + next(rnd, 32);
}

int32_t nextInt(Random* rnd){
    return next(rnd, 32);
}

int32_t nextIntWithRange(Random *rnd, int32_t bound){
    if(bound <= 0){
      // err!
      return 0;
    }
    int32_t r = next(rnd, 31);
    int32_t m = bound - 1;
    if((bound & m) == 0){
        r = (int32_t)((bound * (int64_t)r) >> 31);
    }else{
      int32_t u;
      for(u = r; u - (r = u % bound) + m < 0; u = next(rnd, 31));
    }
    return r;
}

#define DOUBLE_UNIT 0x1.0p-53
double nextDouble(Random *rnd){
  return (((int64_t)(next(rnd, 26))<<27)+next(rnd, 27)) * DOUBLE_UNIT;
}

double nextDoubleXor(){
  static uint32_t y = 2463534242;
  return (((int64_t)((y=xor(y))&0x3ffffff)<<27)+((y=xor(y))&0x7ffffff)) * DOUBLE_UNIT;
}

