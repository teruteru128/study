
#include "bitset.h"
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
#include <pthread.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_QUEUE_SIZE 4 // キューに貯めておけるタスクの最大数
#define NUM_WORKERS 1    // ワーカースレッドの数

// 1つのタスク（メインからワーカーへ渡す素数リストの塊）
typedef struct {
  uint64_t *primes; // 復元された素数の配列（またはビットマップの断片）
  size_t count;     // この塊に含まれる素数の数
} SieveTask;

// スレッド間で共有するタスクキュー
SieveTask task_queue[MAX_QUEUE_SIZE];
int queue_head = 0;
int queue_tail = 0;
int queue_count = 0;
int is_shutdown = 0; // ファイル読み込みがすべて終わったら1にする

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER;

// 各ワーカースレッドの成果物バッファ（スレッドごとに独立、約820KB）
uint64_t worker_outputs[NUM_WORKERS][104858];

/**
 * size->bit単位
 */
uint64_t sieve_search(uint64_t *sieve, size_t size, size_t start) {
  if (start >= size) {
    return (size_t)-1;
  }
  size_t index = start;
  do {
    if ((sieve[unitIndex(index)] & bit(index)) != 0)
      return index;
    index++;
  } while (index < size);
  return (size_t)-1;
}

/**
 * @brief large-sieveを生成してファイルに書き出し
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr,
            "%s <input even number file> <input small sieve file> <output "
            "large sieve file>\n",
            argv[0]);
    return EXIT_FAILURE;
  }
  mpz_t base;
  mpz_init(base);

  // 偶数読み込み
  char *base_filename = argv[1];
  char *smallSievePath = argv[2];
  char *outfilename = argv[3];
  {
    FILE *fin = fopen(base_filename, "r");
    if (fin == NULL) {
      perror("load even number");
      mpz_clear(base);
      return 1;
    }
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    fin = NULL;
  }
  if (mpz_cmp_ui(base, 0) == 0) {
    fprintf(stderr, "why? base is zero.\n");
    mpz_clear(base);
    return 1;
  }
  if (mpz_odd_p(base)) {
    fprintf(stderr, "base is odd.\n");
    mpz_clear(base);
    return 1;
  }
  fprintf(stderr, "even number is %zu bits\n", mpz_sizeinbase(base, 2));

  // 小ふるいオープン
  int smallSieveFD = open(smallSievePath, O_RDONLY);
  if (smallSieveFD < 0) {
    perror("small sieve open failed");
    return 1;
  }
  size_t total_elements = 2147483645ULL;
  size_t file_size = sizeof(uint64_t) + (total_elements * sizeof(uint64_t));

  void *map_start =
      mmap(NULL, file_size, PROT_READ, MAP_SHARED, smallSieveFD, 0);

  uint64_t header = htobe64(*(uint64_t *)map_start);
  uint64_t *file_primes = (uint64_t *)map_start + 1;

  const size_t searchLength = (size_t)(mpz_sizeinbase(base, 2) / 20.0 * 64);
  // Maximum value of the known prime number sieve
  uint64_t maxValOfTheKnownPrimeNumberSieve =
      (uint64_t)total_elements * 64 * 2 + 1;
  uint64_t nextNoti = (uint64_t)(maxValOfTheKnownPrimeNumberSieve * 0.000001);
  uint64_t notiStep = (uint64_t)(maxValOfTheKnownPrimeNumberSieve * 0.000001);
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
  struct tm tm;
  clock_gettime(CLOCK_REALTIME, &startt);
  localtime_r(&startt.tv_sec, &tm);
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
    // ★ ここで配列から取り出すと同時にリトルエンディアンに変換！
    uint64_t element = be64toh(file_primes[i]);
    if (element == (uint64_t)-1) {
      // 全ビット合成数
      continue;
    }
    mask = 1ULL;
    // これで変数 prime には正しい素数（例: 3, 5, 7...）が入ります
    // あとは何事もなかったかのように篩の処理をするだけです
    for (size_t j = 0; j < 64; j++, mask = mask << 1) {
      if ((element & mask) != 0) {
        // 合成数
        continue;
      }
      prime = (i * 64 + j) * 2 + 1;
      start = prime - mpz_fdiv_ui(base, prime);
      if ((start & 1ULL) == 0ULL)
        start += prime;

      while (start < searchLength) {
        largeSieve[unitIndex(start)] |= bit(start);
        start += prime;
      }
      if (prime > nextNoti) {
        bitcnt = 0;
        for (size_t k = 0; k < largeSieveElementNum; k++) {
          bitcnt += __builtin_popcountll(~largeSieve[k]);
        }
        clock_gettime(CLOCK_REALTIME, &startt);
        localtime_r(&startt.tv_sec, &tm);
        strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
        fprintf(stderr,
                "[%s] %" PRIu64
                "(%f %%) done, Remaining prime number candidates: %" PRIu64
                "(%f %%)\n",
                time_buffer, prime,
                (prime * 100.) / maxValOfTheKnownPrimeNumberSieve, bitcnt,
                (double)bitcnt / searchLength);
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

#if 1
  {
    for(size_t i = 0; i < largeSieveElementNum; i++) {
      largeSieve[i] = htobe64(largeSieve[i]);
    }
    FILE *fout = fopen(outfilename, "wb");
    //bs_fileout(fout, largeSieve);
    fwrite(largeSieve, sizeof(uint64_t), largeSieveElementNum, fout);
    fclose(fout);
  }
  // bs_free(largeSieve);
#endif
  mpz_clear(base);
  free(largeSieve);
  close(smallSieveFD);
  munmap(map_start, file_size);
  return 0;
}
