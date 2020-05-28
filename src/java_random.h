
#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef struct random_t
{
  int64_t seed;
} Random;

Random *setSeed(Random *, int64_t);
int32_t next(Random *, int32_t);
int64_t nextLong(Random *);
int32_t nextInt(Random *);
int32_t nextIntWithRange(Random *, int32_t);

#endif
