
#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef struct random_t{
  uint64_t seed;
} Random;

Random *setSeed(Random*, uint64_t);
uint32_t next(Random*, uint32_t);
uint64_t nextLong(Random*);
uint32_t nextInt(Random*);
uint32_t nextIntWithRange(Random*, uint32_t);

#endif

