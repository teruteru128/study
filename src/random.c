
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "random.h"
#include "internal_random.h"

uint32_t xorshift(const uint32_t seed)
{
  uint32_t s = seed;
  s = s ^ (s << 13);
  s = s ^ (s >> 17);
  return s ^ (s << 15);
}

uint64_t xorshift64(const uint64_t seed)
{
  uint64_t s = seed;
  s = s ^ (s << 13);
  s = s ^ (s >> 7);
  return s ^ (s << 17);
}

uint32_t xorshift96(const uint32_t seed1, const uint32_t seed2, const uint32_t seed3)
{
  return 0;
}

uint32_t xorshift128(const uint32_t seed1, const uint32_t seed2, const uint32_t seed3, const uint32_t seed4)
{
  return 0;
}

int read_file(const char *const path, void *buf, const size_t size, const size_t nmemb)
{

  FILE *rnd = fopen(path, "rb");
  if (rnd == NULL)
  {
    perror("fopen rnd");
    return -1;
  }
  size_t r = fread(buf, size, nmemb, rnd);
  if (r < 0)
  {
    perror("fread rnd");
    return -1;
  }
  if (r != nmemb)
  {
    warnx("reading failed");
  }

  (void)fclose(rnd);
  return 0;
}

size_t read_random(void *buf, const size_t size, size_t nmemb, int use_true_random)
{
  char *path[] = {
      "/dev/random",
      "/dev/urandom"};
  read_file(path[!use_true_random], buf, size, nmemb);
  return 0;
}

static int64_t initialScramble(uint64_t seed)
{
  return (seed ^ MULTIPLIER) & MASK;
}

Random *setSeed(Random *rnd, int64_t seed)
{
  rnd->seed = initialScramble(seed);
  return rnd;
}

int32_t next(Random *rnd, int32_t bits)
{
  uint64_t oldseed, nextseed;
  uint64_t seed = rnd->seed;
  oldseed = seed;
  nextseed = (oldseed * MULTIPLIER + ADDEND) & MASK;
  rnd->seed = nextseed;
  return (uint32_t)(nextseed >> (48 - bits));
}

int64_t nextLong(Random *rnd)
{
  return ((int64_t)(next(rnd, 32)) << 32) + next(rnd, 32);
}

int32_t nextInt(Random *rnd)
{
  return next(rnd, 32);
}

int32_t nextIntWithRange(Random *rnd, int32_t bound)
{
  if (bound <= 0)
  {
    // err!
    return 0;
  }
  int32_t r = next(rnd, 31);
  int32_t m = bound - 1;
  if ((bound & m) == 0)
  {
    r = (int32_t)((bound * (int64_t)r) >> 31);
  }
  else
  {
    int32_t u;
    for (u = r; u - (r = u % bound) + m < 0; u = next(rnd, 31))
      ;
  }
  return r;
}

#define DOUBLE_UNIT 0x1.0p-53
double nextDouble(Random *rnd)
{
  return (((int64_t)(next(rnd, 26)) << 27) + next(rnd, 27)) * DOUBLE_UNIT;
}

double nextDoubleXor()
{
  static uint32_t y = 2463534242;
  uint32_t z1 = y = xorshift(y);
  uint32_t z2 = y = xorshift(y);
  return (((int64_t)(z1 & 0x3ffffff) << 27) + (z2 & 0x7ffffff)) * DOUBLE_UNIT;
}

#define NN 312
#define MM 156
#define MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define UM UINT64_C(0xFFFFFFFF80000000) /* Most significant 33 bits */
#define LM UINT64_C(0x7FFFFFFF)         /* Least significant 31 bits */

static uint64_t mt[NN];
static int mti = NN + 1;

void init_genrand64(uint64_t seed)
{
  mt[0] = seed;
  for (mti = 1; mti < NN; mti++)
  {
    mt[mti] = (UINT64_C(6364136223846793005) * (mt[mti - 1] ^ (mt[mti - 1] >> 62)) + 1);
  }
}

void init_by_array64(uint64_t *init_key, size_t key_length)
{
  unsigned int i, j;
  uint64_t k;
  init_genrand64(UINT64_C(19650218));
  i = 1;
  j = 0;
  k = (NN > key_length ? NN : key_length);
  for (; k; k--)
  {
    mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 62)) * UINT64_C(3935559000370003845))) + init_key[j] + j;
    i++;
    j++;
    if (i >= NN)
    {
      mt[0] = mt[NN - 1];
      i = 1;
    }
    if (j >= key_length)
    {
      j = 0;
    }
  }
  for (k = NN - 1; k; k--)
  {
    mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 62)) * UINT64_C(2862933555777941757))) - i; /* non linear */
    i++;
    if (i >= NN)
    {
      mt[0] = mt[NN - 1];
      i = 1;
    }
  }

  mt[0] = UINT64_C(1) << 63; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0, 2^64-1]-interval */
uint64_t genrand64_int64(void)
{
  int i;
  uint64_t x;
  static uint64_t mag01[2] = {UINT64_C(0), MATRIX_A};

  if (mti >= NN)
  { /* generate NN words at one time */

    /* if init_genrand64() has not been called, */
    /* a default initial seed is used     */
    if (mti == NN + 1)
      init_genrand64(UINT64_C(5489));

    for (i = 0; i < NN - MM; i++)
    {
      x = (mt[i] & UM) | (mt[i + 1] & LM);
      mt[i] = mt[i + MM] ^ (x >> 1) ^ mag01[(int)(x & UINT64_C(1))];
    }
    for (; i < NN - 1; i++)
    {
      x = (mt[i] & UM) | (mt[i + 1] & LM);
      mt[i] = mt[i + (MM - NN)] ^ (x >> 1) ^ mag01[(int)(x & UINT64_C(1))];
    }
    x = (mt[NN - 1] & UM) | (mt[0] & LM);
    mt[NN - 1] = mt[MM - 1] ^ (x >> 1) ^ mag01[(int)(x & UINT64_C(1))];

    mti = 0;
  }

  x = mt[mti++];

  x ^= (x >> 29) & UINT64_C(0x5555555555555555);
  x ^= (x << 17) & UINT64_C(0x71D67FFFEDA60000);
  x ^= (x << 37) & UINT64_C(0xFFF7EEE000000000);
  x ^= (x >> 43);

  return x;
}

/* generates a random number on [0, 2^63-1]-interval */
int64_t genrand64_int63(void)
{
  return (int64_t)(genrand64_int64() >> 1);
}

/* generates a random number on [0,1]-real-interval */
double genrand64_real1(void)
{
  return (genrand64_int64() >> 11) * (1.0 / 9007199254740991.0);
}

/* generates a random number on [0,1)-real-interval */
double genrand64_real2(void)
{
  return (genrand64_int64() >> 11) * (1.0 / 9007199254740992.0);
}

/* generates a random number on (0,1)-real-interval */
double genrand64_real3(void)
{
  return ((genrand64_int64() >> 12) + 0.5) * (1.0 / 4503599627370496.0);
}

int nextBytes(unsigned char *buf, size_t len)
{
    char *inf = "/dev/urandom";
    FILE *in = fopen(inf, "rb");
    if (in == NULL)
    {
        return EXIT_FAILURE;
    }

    size_t r = fread(buf, 1, len, in);

    if (len != r)
    {
        perror("fread");
        fclose(in);
        return EXIT_FAILURE;
    }
    int i = fclose(in);

    if (i != 0)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// 
// nextもrandomも使われているので適当にpとする
int64_t p(int64_t seed) {
  return (seed * 0x5DEECE66DL + 0xBL) & 0xFFFFFFFFFFFFL;
}

int64_t pInverse(int64_t seed) {
  return (seed - 0xBL) * 0xDFE05BCB1365L & 0xFFFFFFFFFFFFL;
}

int64_t initializeSeed(int64_t seed) {
  return (seed ^ 0x5DEECE66DL) & 0xFFFFFFFFFFFFL;
}
