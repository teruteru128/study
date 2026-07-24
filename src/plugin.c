
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
uint64_t s[4];

// ビット回転の補助関数
static inline uint64_t rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

// 64ビットの高品質な整数乱数を生成
uint64_t next(void) {
  const uint64_t result = rotl(s[1] * 5, 7) * 9;
  const uint64_t t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;
  s[3] = rotl(s[3], 45);

  return result;
}

// 0.0 から 1.0 未満の double 型乱数を生成（53ビットの仮数部を完全に満たす）
double next_double(void) { return (next() >> 11) * (1.0 / 9007199254740992.0); }

void *nextCumshoot(char *msg) {
  srand(time(NULL));
  int ret = getrandom(s, sizeof(s), GRND_NONBLOCK);
  if (ret < 0) {
    // 万が一ファイルが開けなかった場合のフォールバック（時間を使用）
    s[0] = 0x123456789ABCDEF0ULL;
    s[1] = (uint64_t)time(NULL);
    s[2] = s[1] << 16;
    s[3] = s[1] >> 16;
  }
  // 範囲設定: log(0.8) 〜 log(13.0)
  double min_val = log(0.8);
  double max_val = log(13.0);

  // 乱数の生成と変換
  double r0;
  double r1;
  double result0;
  double result1;

  int num;
  size_t count = 0;
  do {
  	r0  = next_double();
    result0 = min_val + r0 * (max_val - min_val);
  	r1  = next_double();
    result1 = r1 * N;
    count++;

    if (result1 < 1) {
      printf("!!! %zu: %f\n", count, exp(result0) * 3000);
    } else {
      printf("%zu: %f\n", count, exp(result0));
    }
  } while (result1 >= 1);

  return NULL;
}
