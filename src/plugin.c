
#include <bits/time.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <malloc.h>
#include <math.h>
#include <plugin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <time.h>
#include <unistd.h>

void *aaaa_aaaa(void *message) {
  printf("[Optional Feature] Plugin running: %s\n", (char *)message);
  void *handle1 = dlopen("libm.so.6", RTLD_LAZY);
  void *handle2 = dlopen("m", RTLD_LAZY);
  printf("libm.so: %p, m: %p\n", handle1, handle2);
  printf("%s\n", handle1 == handle2 ? "true" : "false");
end:
  if (handle1)
    dlclose(handle1);
  if (handle2)
    dlclose(handle2);
  return NULL;
}
#define N 3000

// xoshiro256** の内部状態（4つの64ビット整数）
uint64_t state[4];

// ビット回転の補助関数
static inline uint64_t rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

// 64ビットの高品質な整数乱数を生成
uint64_t next(void) {
  const uint64_t result = rotl(state[1] * 5, 7) * 9;
  const uint64_t t = state[1] << 17;

  state[2] ^= state[0];
  state[3] ^= state[1];
  state[1] ^= state[2];
  state[0] ^= state[3];

  state[2] ^= t;
  state[3] = rotl(state[3], 45);

  return result;
}

// 0.0 から 1.0 未満の double 型乱数を生成（53ビットの仮数部を完全に満たす）
double next_double(void) { return (next() >> 11) * (1.0 / 9007199254740992.0); }

uint64_t splitmix64_next(uint64_t *state) {
  uint64_t z = (*state += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

void *nextCumshoot(char *msg) {
  srand(time(NULL));
  int ret = getrandom(state, sizeof(state), 0);
  if (ret < sizeof(state)) {
    // 万が一ファイルが開けなかった場合のフォールバック（時間を使用）
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
      ts.tv_sec = 0;
      ts.tv_nsec = 0;
    }

    // 1. 秒（64bit）とナノ秒（64bitにキャスト）を準備
    uint64_t sec = (uint64_t)ts.tv_sec;
    uint64_t nsec = (uint64_t)ts.tv_nsec;

    // 2. ナノ秒の情報を上位ビット側へシフトし、秒と結合する
    //    （ナノ秒は最大 999,999,999 なので30ビットあれば収まります）
    uint64_t combined = sec ^ (nsec << 32);

    // 3.
    // 結合した値をさらに「大きな素数」で乗算し、ビットを混ぜ合わせる（簡易ハッシュ化）
    //    これにより、近い時間であってもビットパターンが完全にバラバラになります
    uint64_t seed = combined * 0x9e3779b97f4a7c15ULL;
    state[0] = splitmix64_next(&seed);
    state[1] = splitmix64_next(&seed);
    state[2] = splitmix64_next(&seed);
    state[3] = splitmix64_next(&seed);
  }
  // 範囲設定: log(0.8) 〜 log(13.0)
  const double min_val = log(0.8);
  const double max_val = log(3) * 6. * log(10);
  const double diff_val = max_val - min_val;
  double d_max_val = log(1500000.0 / 3000.0);

  // 乱数の生成と変換
  double r0;
  double r1;
  double result0;
  double result1;
  double explosive = -1.0;

  size_t count = 0;
  while(count < 3) {
    r0 = next_double();
    result0 = min_val + r0 * diff_val;
    double cumshoot = exp(result0);
    if (cumshoot >= 1200000.) {
      count++;
    }
    printf("%zu: %f\n", count, cumshoot);
  }

  return NULL;
}

void *genentropypool(char *msg) {
  unsigned char buffer[64];
  getrandom(buffer, sizeof(buffer), GRND_RANDOM);
  FILE *out = fopen("entropy_pool.bin", "wb");
  if (out == NULL) {
    perror("ファイルオープン失敗");
    return NULL;
  }
  size_t num = fwrite(buffer, 1, 64, out);

  fclose(out);
  return NULL;
}

void *prin(char *msg) {
  printf("%" PRIu64 "\n", (uint64_t)strlen(msg));
  return msg;
}
