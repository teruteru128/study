
#ifndef RANDOM_H
#define RANDOM_H

typedef struct random_t{
  int64_t seed;
} Random;

uint32_t xor(const uint32_t);
uint64_t xor64(const uint64_t);
uint32_t xor96(const uint32_t, const uint32_t, const uint32_t);
uint32_t xor128(const uint32_t, const uint32_t, const uint32_t, const uint32_t);

int get_random(const char* const, void*, const size_t, const size_t);

Random *setSeed(Random*, int64_t);
int32_t next(Random*, int32_t);
int64_t nextLong(Random*);
int32_t nextInt(Random*);
int32_t nextIntWithRange(Random*, int32_t);
double nextDouble(Random*);

#endif

