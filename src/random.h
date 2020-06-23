
#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include <inttypes.h>

typedef struct random_t
{
  int64_t seed;
} Random;

uint32_t xorshift (const uint32_t);
uint64_t xorshift64(const uint64_t);
uint32_t xorshift96(const uint32_t, const uint32_t, const uint32_t);
uint32_t xorshift128(const uint32_t, const uint32_t, const uint32_t, const uint32_t);

int read_file(const char *const, void *, const size_t, const size_t);
size_t read_random(void *buf, const size_t size, size_t nmemb, int use_true_random);

Random *setSeed(Random *, int64_t);
int32_t next(Random *, int32_t);
int64_t nextLong(Random *);
int32_t nextInt(Random *);
int32_t nextIntWithRange(Random *, int32_t);
double nextDouble(Random *);

void init_genrand64(uint64_t);
void init_by_array64(uint64_t *, size_t);
uint64_t genrand64_int64(void);
int64_t genrand64_int63(void);
double genrand64_real1(void);
double genrand64_real2(void);
double genrand64_real3(void);
int nextBytes(unsigned char *buf, size_t len);

// java.util.Random 内部状態計算ユーティリティ
int64_t p(int64_t);
int64_t pInverse(int64_t);
int64_t initializeSeed(int64_t);

#endif
