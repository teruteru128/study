/**
 * @file fiblucas_tf.c
 * @brief フィボナッチ数F(n)・リュカ数L(n)の原始素因数を、形を絞った試し割りで探す。
 *
 * F(n)で初めて現れる素因数pは Z(p) = n (Z(p)はpを割る最小のフィボナッチ数の添字)を満たし、
 * Z(p) | p - (5/p) なので p ≡ ±1 (mod n)。pは奇数なので、nが奇数なら p ≡ ±1 (mod 2n)。
 * L(n)の素因数はF(2n)の原始素因数なので、L(n)は添字2nのF(n)と同じ扱いになる。
 * そこで M = (Zが偶数ならZ、奇数なら2Z) として、候補を p = kM ± 1 に限る。
 *
 * 候補ごとに巨大な数を割るのではなく、倍加公式で F(n) mod p を直接計算するので、
 * 1候補の手間は数の大きさに関係なく log2(n) 回程度の64bit剰余乗算で済む
 * (GIMPSの試し割りと同じ考え方)。剰余乗算はモンゴメリ乗算で行うため p < 2^63 に限る。
 *
 * 候補は小さな素数で篩ってから判定する。篩の上限はM-2を超えないように抑えるので、
 * 候補自身が篩の素数と一致して消えることはない。
 *
 * 使い方: fiblucas_tf <F|L> <n> <pmin> <pmax> [チェックポイントファイル]
 *   チェックポイントファイルを渡すと、区間ごとに進んだkを書き出し、再実行時にそこから再開する。
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned __int128 u128;

#define SEGMENT (1u << 18)       /* 1区間あたりのkの個数 */
#define SIEVE_LIMIT_MAX 1000000u /* 篩に使う素数の上限 */

typedef struct {
  uint64_t p;
  uint64_t pinv; /* -p^{-1} mod 2^64 */
  uint64_t r1;   /* 2^64 mod p (モンゴメリ表現の1) */
} mont_t;

static inline uint64_t mont_mul(uint64_t a, uint64_t b, const mont_t *m) {
  u128 t = (u128)a * b;
  uint64_t q = (uint64_t)t * m->pinv;
  /* p < 2^63 なので t + q*p < 2^128 に収まり、結果は 2p 未満 */
  uint64_t u = (uint64_t)((t + (u128)q * m->p) >> 64);
  return u >= m->p ? u - m->p : u;
}

static inline uint64_t add_mod(uint64_t a, uint64_t b, uint64_t p) {
  uint64_t s = a + b; /* p < 2^63 なので溢れない */
  return s >= p ? s - p : s;
}

static inline uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t p) {
  return a >= b ? a - b : a + p - b;
}

static void mont_init(mont_t *m, uint64_t p) {
  uint64_t inv = p; /* ニュートン法で p^{-1} mod 2^64 */
  for (int i = 0; i < 5; i++) inv *= 2 - p * inv;
  m->p = p;
  m->pinv = -inv;
  m->r1 = (uint64_t)(((u128)1 << 64) % p);
}

static uint64_t to_mont(uint64_t x, const mont_t *m) {
  return (uint64_t)(((u128)x << 64) % m->p);
}

/** F(n) mod p と F(n+1) mod p をモンゴメリ表現で求める(倍加公式) */
static void fib_pair(uint64_t n, const mont_t *m, uint64_t *fn, uint64_t *fn1) {
  uint64_t a = 0, b = m->r1; /* (F(0), F(1)) */
  for (int i = 63 - __builtin_clzll(n); i >= 0; i--) {
    /* F(2k) = F(k)(2F(k+1) - F(k)), F(2k+1) = F(k)^2 + F(k+1)^2 */
    uint64_t c = mont_mul(a, sub_mod(add_mod(b, b, m->p), a, m->p), m);
    uint64_t d = add_mod(mont_mul(a, a, m), mont_mul(b, b, m), m->p);
    if ((n >> i) & 1) {
      a = d;
      b = add_mod(c, d, m->p);
    } else {
      a = c;
      b = d;
    }
  }
  *fn = a;
  *fn1 = b;
}

/** 64bit向けの決定的Miller-Rabin(ヒットした候補が素数かどうかの確認用) */
static int is_prime64(uint64_t p) {
  if (p < 2) return 0;
  for (uint64_t r = 2; r < 40; r++)
    if (p % r == 0) return p == r;
  static const uint64_t bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
  mont_t m;
  mont_init(&m, p);
  uint64_t d = p - 1;
  int s = __builtin_ctzll(d);
  d >>= s;
  uint64_t one = m.r1, minus_one = p - m.r1;
  for (size_t i = 0; i < sizeof bases / sizeof bases[0]; i++) {
    uint64_t a = bases[i] % p;
    if (a == 0) continue;
    uint64_t x = one, base = to_mont(a, &m);
    for (uint64_t e = d; e; e >>= 1) {
      if (e & 1) x = mont_mul(x, base, &m);
      base = mont_mul(base, base, &m);
    }
    if (x == one || x == minus_one) continue;
    int ok = 0;
    for (int j = 1; j < s && !ok; j++) {
      x = mont_mul(x, x, &m);
      ok = x == minus_one;
    }
    if (!ok) return 0;
  }
  return 1;
}

/** a^{-1} mod r (rは素数、a≠0 mod r) */
static uint32_t inv_mod(uint64_t a, uint32_t r) {
  int64_t t = 0, nt = 1, rr = r, nr = (int64_t)(a % r);
  while (nr) {
    int64_t q = rr / nr, tmp;
    tmp = t - q * nt, t = nt, nt = tmp;
    tmp = rr - q * nr, rr = nr, nr = tmp;
  }
  return (uint32_t)(t < 0 ? t + r : t);
}

static void save_checkpoint(const char *path, uint64_t k) {
  char tmp[4096];
  snprintf(tmp, sizeof tmp, "%s.tmp", path);
  FILE *fp = fopen(tmp, "w");
  if (fp == NULL) return;
  fprintf(fp, "%" PRIu64 "\n", k);
  fclose(fp);
  rename(tmp, path);
}

int main(int argc, char *argv[]) {
  if (argc < 5 || (argv[1][0] != 'F' && argv[1][0] != 'L')) {
    fprintf(stderr, "使い方: %s <F|L> <n> <pmin> <pmax> [チェックポイントファイル]\n", argv[0]);
    return 1;
  }
  const int lucas = argv[1][0] == 'L';
  const uint64_t n = strtoull(argv[2], NULL, 10);
  const uint64_t pmin = strtoull(argv[3], NULL, 10);
  const uint64_t pmax = strtoull(argv[4], NULL, 10);
  const char *ckpt = argc >= 6 ? argv[5] : NULL;
  if (n < 3 || pmax >= (UINT64_C(1) << 63) || pmin > pmax) {
    fprintf(stderr, "n >= 3、pmin <= pmax < 2^63 にしてください\n");
    return 1;
  }

  const uint64_t z = lucas ? 2 * n : n;
  const uint64_t M = (z & 1) ? 2 * z : z;
  uint64_t k = (pmin > M ? (pmin - 1) / M : 1);
  const uint64_t kend = (pmax + 1) / M + 1; /* この値は含まない */
  if (ckpt != NULL) {
    FILE *fp = fopen(ckpt, "r");
    uint64_t saved;
    if (fp != NULL && fscanf(fp, "%" SCNu64, &saved) == 1 && saved > k) {
      k = saved;
      printf("チェックポイントから再開: k = %" PRIu64 "\n", k);
    }
    if (fp != NULL) fclose(fp);
  }

  /* 篩に使う小さな素数(Mを割る素数は kM±1 を割らないので除く) */
  const uint32_t limit = M - 2 < SIEVE_LIMIT_MAX ? (uint32_t)(M - 2) : SIEVE_LIMIT_MAX;
  char *composite = calloc(limit + 1, 1);
  uint32_t *primes = malloc(sizeof(uint32_t) * (limit / 2 + 1));
  uint32_t *minv = malloc(sizeof(uint32_t) * (limit / 2 + 1));
  size_t np = 0;
  for (uint32_t i = 3; i <= limit; i += 2) {
    if (composite[i]) continue;
    for (uint64_t j = (uint64_t)i * i; j <= limit; j += 2 * i) composite[j] = 1;
    if (M % i == 0) continue;
    primes[np] = i;
    minv[np] = inv_mod(M, i);
    np++;
  }
  free(composite);

  printf("%s(%" PRIu64 ") の原始素因数を p = %" PRIu64 "k ± 1 の形で探す: p ∈ [%" PRIu64 ", %" PRIu64 "]\n",
         lucas ? "L" : "F", n, M, pmin, pmax);
  printf("k ∈ [%" PRIu64 ", %" PRIu64 ")、篩の素数 %zu 個(上限 %u)\n", k, kend, np, limit);
  fflush(stdout);

  unsigned char *sieve[2] = {malloc(SEGMENT), malloc(SEGMENT)}; /* [0]: kM+1, [1]: kM-1 */
  uint64_t tested = 0, found = 0;
  const time_t start = time(NULL);
  time_t last_report = start;

  while (k < kend) {
    const uint32_t len = kend - k < SEGMENT ? (uint32_t)(kend - k) : SEGMENT;
    memset(sieve[0], 1, len);
    memset(sieve[1], 1, len);
    for (size_t i = 0; i < np; i++) {
      const uint32_t r = primes[i];
      /* kM+1 ≡ 0 ⇔ k ≡ -M^{-1}、kM-1 ≡ 0 ⇔ k ≡ M^{-1} (mod r) */
      const uint32_t t[2] = {minv[i] ? r - minv[i] : 0, minv[i]};
      const uint32_t kmod = (uint32_t)(k % r);
      for (int s = 0; s < 2; s++) {
        for (uint64_t j = (t[s] + r - kmod) % r; j < len; j += r) sieve[s][j] = 0;
      }
    }
    for (uint32_t j = 0; j < len; j++) {
      for (int s = 0; s < 2; s++) {
        if (!sieve[s][j]) continue;
        const uint64_t p = s == 0 ? (k + j) * M + 1 : (k + j) * M - 1;
        if (p < pmin || p > pmax) continue;
        mont_t m;
        mont_init(&m, p);
        uint64_t fn, fn1;
        fib_pair(n, &m, &fn, &fn1);
        /* L(n) = 2F(n+1) - F(n) */
        const uint64_t v = lucas ? sub_mod(add_mod(fn1, fn1, p), fn, p) : fn;
        tested++;
        if (v == 0) {
          found++;
          printf("FOUND %" PRIu64 " = %" PRIu64 "*%" PRIu64 "%s1 divides %s(%" PRIu64 ") 素数: %s\n", p, M,
                 k + j, s == 0 ? "+" : "-", lucas ? "L" : "F", n, is_prime64(p) ? "yes" : "no");
          fflush(stdout);
        }
      }
    }
    k += len;
    if (ckpt != NULL) save_checkpoint(ckpt, k);
    const time_t now = time(NULL);
    if (now - last_report >= 600 || k >= kend) {
      const double el = difftime(now, start);
      printf("進捗: k = %" PRIu64 " (p ≈ %.3e) 判定 %" PRIu64 " 件、%.0f 秒、%.3g 件/秒\n", k, (double)k * M,
             tested, el, el > 0 ? tested / el : 0.0);
      fflush(stdout);
      last_report = now;
    }
  }
  printf("完了: 判定 %" PRIu64 " 件、因数 %" PRIu64 " 件\n", tested, found);
  free(sieve[0]);
  free(sieve[1]);
  free(primes);
  free(minv);
  return 0;
}
