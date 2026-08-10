
#include "bitset.h"
#include "large_sieve_io.h"
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "bitsieve.h"
#include "timeutil.h"
#include <endian.h>
#include <gmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * @brief large-sieveを生成してファイルに書き出し
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr,
            "%s <input even number file> <input small sieve file> <output "
            "large sieve file>\n",
            argv[0]);
    return EXIT_FAILURE;
  }
  mpz_t base;
  mpz_init(base);

  char *base_filename = argv[1];
  char *smallSievePath = argv[2];
  char *outfilename = argv[3];

  if (load_even_base(base_filename, base) != 0) {
    mpz_clear(base);
    return 1;
  }
  fprintf(stderr, "even number is %zu bits\n", mpz_sizeinbase(base, 2));

  struct SmallSieve smallSieve;
  if (small_sieve_open(smallSievePath, &smallSieve) != 0) {
    mpz_clear(base);
    return 1;
  }
  size_t total_elements = smallSieve.total_elements;
  uint64_t *file_primes = smallSieve.primes;

  const size_t searchLength = (size_t)(mpz_sizeinbase(base, 2) / 20.0 * 64);
  // Maximum value of the known prime number sieve
  uint64_t maxValOfTheKnownPrimeNumberSieve =
      (uint64_t)total_elements * 64 * 2 + 1;
  // 通知1回ごとにlargeSieve全体をpopcountし直すため、既知素数篩がどれだけ
  // 大きくても通知回数がおおよそ一定(NOTI_COUNT回程度)に収まるようにする
  const uint64_t NOTI_COUNT = 2000;
  uint64_t notiStep = maxValOfTheKnownPrimeNumberSieve / NOTI_COUNT;
  if (notiStep == 0)
    notiStep = 1;
  uint64_t nextNoti = notiStep;
  fprintf(stderr, "large sieve size: %zu bits\n", searchLength);
  size_t largeSieveElementNum = ((searchLength - 1) / 64) + 1;
  fprintf(stderr, "large sieve elements: %zu elements\n", largeSieveElementNum);
  fprintf(stderr, "small sieve size: %zu bits\n", total_elements * 64);
  uint64_t *largeSieve = calloc(largeSieveElementNum, sizeof(uint64_t));
  if (largeSieve == NULL) {
    perror("large sieve malloc error");
    exit(1);
  }
  // printf("%lu\n", searchLength);
  // struct BitSieve *largeSieve = bs_new();
  struct timespec startt;
  struct timespec now;
  struct tm tm;
  clock_gettime(CLOCK_REALTIME, &now);
  localtime_r(&now.tv_sec, &tm);
  char time_buffer[64];
  strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
  fprintf(stderr, "[%s] 基準偶数に対するlarge-sieveの生成を開始します...\n",
          time_buffer);
  clock_gettime(CLOCK_MONOTONIC, &startt);
  // ファイル読み進めながら篩い分けするの〜？
  // bs_initInstance(largeSieve, base, searchLength);

  uint64_t mask = 1ULL;
  uint64_t start = 0;
  uint64_t prime = 3;
  uint64_t bitcnt = 0;
  for (size_t i = 0; i < total_elements; i++) {
    uint64_t element = be64toh(file_primes[i]);
    if (element == (uint64_t)-1) {
      // 全ビット合成数
      continue;
    }
    mask = 1ULL;
    for (size_t j = 0; j < 64; j++, mask = mask << 1) {
      if ((element & mask) != 0) {
        // 合成数
        continue;
      }
      prime = (i * 64 + j) * 2 + 1;
      start = prime - mpz_fdiv_ui(base, prime);
      if ((start & 1ULL) == 0ULL)
        start += prime;

      start = (start - 1) / 2;
      while (start < searchLength) {
        largeSieve[unitIndex(start)] |= bit(start);
        start += prime;
      }
      if (prime > nextNoti) {
        bitcnt = 0;
        for (size_t k = 0; k < largeSieveElementNum; k++) {
          bitcnt += __builtin_popcountll(~largeSieve[k]);
        }
        clock_gettime(CLOCK_REALTIME, &now);
        localtime_r(&now.tv_sec, &tm);
        strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
        fprintf(stderr,
                "[%s] %" PRIu64
                "(%f %%) done, Remaining prime number candidates: %" PRIu64
                "(%f %%)\n",
                time_buffer, prime,
                (prime * 100.) / maxValOfTheKnownPrimeNumberSieve, bitcnt,
                (double)bitcnt / searchLength * 100.);
        nextNoti += notiStep;
      }
    }
  }

  struct timespec finish;
  clock_gettime(CLOCK_MONOTONIC, &finish);
  struct timespec diff;
  difftimespec(&diff, &finish, &startt);
  clock_gettime(CLOCK_REALTIME, &finish);
  localtime_r(&finish.tv_sec, &tm);
  strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
  fprintf(stderr, "%s: 篩の初期化を完了しました. (%ld.%09lds)\n", time_buffer,
          diff.tv_sec, diff.tv_nsec);

  int writeResult = write_large_sieve(outfilename, largeSieve, largeSieveElementNum, searchLength);

  mpz_clear(base);
  free(largeSieve);
  small_sieve_close(&smallSieve);
  return writeResult == 0 ? 0 : 1;
}
